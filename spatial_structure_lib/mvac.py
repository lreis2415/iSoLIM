import pyopencl as cl
import pkg_resources
import numpy as np
import sys

class MultiVarAutoCor_cl:
    def __init__(self,
    cov_arr:np.ndarray,
    datatypes:list=[],
    nodata:int=-9999,
    alpha:float=2,
    threshold: float = 1

    ):
        """Initialize class and create opencl program code
        @param cov_arr:np.ndarray: collection of values of covariates within the study area,
                                e.g. the array of covariate values of a 4*3 extent with 2 layers of covariates:
                                np.array([
                                                [
                                                    [0, 4, 1, 3],
                                                    [0, 5, 2, 1],
                                                    [1, 6, 9 ,4]
                                                ],
                                                [
                                                    [52,46,91,73],
                                                    [80,65,32,61],
                                                    [51,46,61,84]
                                                ]
                                            ])
        @param nodata: nodata value of each covariate layer, single value or list
        @param alpha: the distance decay coefficient, default is 2
        """
        self._covarr = cov_arr
        self._threshold = threshold
        if len(cov_arr.shape)!=3:
            print('ERROR: please provide a 3d numpy array for cov_arr')
            exit()
        self._dim, self._y, self._x  = cov_arr.shape
        if isinstance(nodata,np.ndarray) or isinstance(nodata,list):
            if len(nodata)!=self._dim:
                print('ERROR: if provided nodata is list or array, please ensure the length is consistent with the number of layers')
                exit()
            self._nodata = np.array(nodata)
        elif isinstance(nodata,int) or isinstance(nodata,float):
            self._nodata=np.full(self._dim,nodata)
        
        for i in range(self._dim):
            self._covarr[i][self._covarr[i]==self._nodata[i]]=np.nan
        
        # datatypes initialization
        self._datatypes = []
        str_types="{"
        if datatypes==[]:
            for i in range(self._dim):
                self._datatypes.append(0)
                str_types = str_types+"CONTINUOUS,"
        elif len(datatypes)!=self._dim:
            print('Warning: if provided datatypes are list, please ensure the length is consistent with the number of layers')
            exit()
        else:
            for i in range(self._dim):
                if datatypes[i] == "CATEGORICAL" or datatypes[i]==1:
                    self._datatypes.append(1)
                    str_types = str_types+"CATEGORICAL,"
                elif datatypes[i] == "CONTINUOUS" or datatypes[i]==0:
                    self._datatypes.append(0)
                    str_types = str_types+"CONTINUOUS,"
                else:
                    print("WARNING: wrong format of provided dataypes, treated as continuous data")
                    self._datatypes.append(0)
                    str_types = str_types+"CONTINUOUS,"
            
        str_types = str_types[:-1]+"}"
        
        # c code for opencl
        self._prg_code_main = '''
        #define dist_decay '''+str(alpha)+'''
        #define dim '''+str(self._dim)+'''
        #define threshold '''+str(self._threshold)+'''
        enum datatype{CONTINUOUS, CATEGORICAL};
        __kernel void simi(__global const int *loc_label_i, __global const int *loc_label_j, __global const int *loc_dict_x, __global const int *loc_dict_y,
                        __global float *simi, __global float *dist_weight,
                        __global const float *feature, __global float *std_mean_ass){
            unsigned int gid = get_global_id(0);
            enum datatype types[] = '''+str_types+''';

            int label_i = loc_label_i[gid];
            int label_j = loc_label_j[gid];
            
            int xi = loc_dict_x[label_i];
            int yi = loc_dict_y[label_i];
            int xj = loc_dict_x[label_j];
            int yj = loc_dict_y[label_j];
            
            /*float dist=sqrt(pow((xi-xj)*(xi-xj)+(yi-yj)*(yi-yj),dist_decay/2.0));
            if (dist > threshold) dist_weight[gid]=0;
            else dist_weight[gid]=1;*/
            dist_weight[gid] = 1.0/pow((xi-xj)*(xi-xj)+(yi-yj)*(yi-yj),dist_decay/2.0);

            float simi_sum = 0;
            float cov_w_sum = 0;
            float simi_min = 2;
            // calculate the covariate between i and j
            for(unsigned int k = 0; k<dim;k++){
                // calculate the similarity on k-th covariate
                float covi = feature[label_i*dim+k];
                float covj = feature[label_j*dim+k];
                float simi;
                if ((covi > -9998) && (covj > -9998)) {
                    if (types[k]==CONTINUOUS){
                        float dev_sq = (covi-covj)*(covi-covj);
                        float sij=exp(-dev_sq*0.5f/std_mean_ass[k+dim]*(covi*covi+std_mean_ass[k]+std_mean_ass[k+dim*3]-2.0f*covi*std_mean_ass[k+dim*2]));
                        float sji=exp(-dev_sq*0.5f/std_mean_ass[k+dim]*(covj*covj+std_mean_ass[k]+std_mean_ass[k+dim*3]-2.0f*covj*std_mean_ass[k+dim*2])); //ipsm, Zhu et al., 2015
                        simi = (sij+sji)*0.5;  
                    } else {
                        if (fabs(covi-covj)<0.0001) simi = 1;
                        else simi=0;
                    }
                    if(simi<simi_min || simi_min>1) {
                        simi_min = simi;
                        //simi_sum += (sij+sji)*0.5*cov_weights[k];
                        //cov_w_sum += cov_weights[k];
                    }
                }
            }
            float covi = feature[label_i*dim];
            float covj = feature[label_j*dim];
            simi[gid]=simi_min;//simi_sum/cov_w_sum;
        }
        '''

    def _getValidLoc(self):
        loc_x = []
        loc_y = []
        mask = np.zeros((self._y, self._x))
        for i in range(self._y):
            for j in range(self._x):
                covs = self._covarr[:,i,j]
                #if not np.isnan(covs).any():
                if (~np.isnan(covs)).any() and (~(covs==self._nodata)).any():
                    loc_y.append(i)
                    loc_x.append(j)
                    mask[i,j]=1
        return loc_x, loc_y, mask

    def run(self):
        platform = cl.get_platforms()[0]  # Select the first platform [0]
        device = platform.get_devices()[0]  # Select the first device on this platform [0]
        ctx = cl.Context([device]) 
        #ctx = cl.create_some_context()
        queue = cl.CommandQueue(ctx)
        prg = cl.Program(ctx, self._prg_code_main).build()
        stds=[]
        means=[]
        mean_sq = []
        std_sq=[]
        std_sq4=[]
        for i in range(self._dim):
            stds.append(np.nanstd(self._covarr[i]))
            means.append(np.nanmean(self._covarr[i]))
            std_sq.append(pow(stds[i],2))
            mean_sq.append(pow(means[i],2))
            std_sq4.append(pow(stds[i],4))
        std_mean_ass = []
        std_mean_ass.extend(std_sq)
        std_mean_ass.extend(std_sq4)
        std_mean_ass.extend(means)
        std_mean_ass.extend(mean_sq)
        loc_dict_x, loc_dict_y, mask = self._getValidLoc()
        cov_ar_list=[]
        for i in range(self._dim):
            self._covarr[i][self._covarr[i]==self._nodata[i]]=-9999
            cov_ar_list.append(self._covarr[i][mask==1])
        cov_arr_2d_flat = np.column_stack(cov_ar_list)
        cov_arr_1d= cov_arr_2d_flat.flatten()
        cov_arr_1d[np.isnan(cov_arr_1d)] = -9999
        mf = cl.mem_flags
        loc_dict_x_g = cl.Buffer(ctx, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=np.array(loc_dict_x))
        loc_dict_y_g = cl.Buffer(ctx, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=np.array(loc_dict_y))
        feature_g = cl.Buffer(ctx, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=cov_arr_1d.astype(np.float32))
        std_mean_ass_g = cl.Buffer(ctx, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=np.array(std_mean_ass).astype(np.float32))
        queue.finish()
        
        # iterate over location pairs
        loc_itr = 0
        sum_w = 0
        sum_s = 0
        sum_ws = 0
        sum_n = 0
        while loc_itr< (len(loc_dict_x)-1):
            loc_labels_i=[]
            loc_labels_j=[]
            k=0
            for i in range(loc_itr, len(loc_dict_x)):
                loc_labels_i.extend([i]*len(range(i+1,len(loc_dict_x))))
                loc_labels_j.extend(range(i+1,len(loc_dict_x)))
                k=k+len(range(i+1,len(loc_dict_x)))
                if(k>1000000):
                    break
            loc_itr = i+1
            #sys.stdout.write("\r{0}".format((float(loc_itr)/len(loc_dict_x))*100))
            #sys.stdout.flush()
            loc_labels_i = np.array(loc_labels_i)
            loc_labels_j = np.array(loc_labels_j)
            loc_label_i_g = cl.Buffer(ctx, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=loc_labels_i)
            loc_label_j_g = cl.Buffer(ctx, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=loc_labels_j)
            simi_g = cl.Buffer(ctx, mf.WRITE_ONLY, loc_labels_i.astype(np.float32).nbytes)
            dist_weights_g = cl.Buffer(ctx, mf.WRITE_ONLY, loc_labels_i.astype(np.float32).nbytes)
        

            #knl = prg.simi  # Use this Kernel object for repeated calls
            completeEvent = prg.simi(queue, loc_labels_i.shape, None, loc_label_i_g, loc_label_j_g, loc_dict_x_g, loc_dict_y_g,
                        simi_g, dist_weights_g,
                        feature_g, std_mean_ass_g)
            events = [ completeEvent ]
            queue.finish()
            # Wait for the queue to be completely processed.
            simi_np = np.empty_like(loc_labels_i.astype(np.float32))
            dist_weights_np = np.empty_like(loc_labels_i.astype(np.float32))
            cl.enqueue_copy(queue, simi_np, simi_g,wait_for = events)
            cl.enqueue_copy(queue, dist_weights_np, dist_weights_g)
            sum_w = sum_w+np.nansum(dist_weights_np[~np.isnan(simi_np)])
            sum_s = sum_s + np.nansum(simi_np)
            sum_ws = sum_ws + np.nansum(simi_np*dist_weights_np)
            sum_n = sum_n + np.count_nonzero(~np.isnan(simi_np))
            loc_label_i_g.release()
            loc_label_j_g.release()
            simi_g.release()
            dist_weights_g.release()

        loc_dict_x_g.release()
        loc_dict_y_g.release()
        feature_g.release()
        std_mean_ass_g.release()
        if sum_s==0:
            self._mvac = 9999
            self._weighted_simi=0
            return 9999
        self._mvac = (sum_ws/sum_w)/(sum_s/sum_n)
        self._weighted_simi = sum_ws/sum_w
        return self._mvac