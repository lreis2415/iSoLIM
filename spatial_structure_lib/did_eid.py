from scipy.optimize import least_squares

class independentDegree:
    def __init__(self,
    samples_covs:np.ndarray,
    samples_coords:np.ndarray,
    target:list,
    stds:list=[],
    means:list=[],
    mean_sq:list=[],
    std_sq:list=[],
    std_sq4:list=[],
    datatypes:list=[],
    lag:int=50,
    draw:bool=False):
        if len(samples_covs.shape)!=2:
            print('ERROR: Wrong sample_covs format')
            exit()
        self.sample_num, self.cov_num = samples_covs.shape
        if self.sample_num != sample_coords.shape[0]:
            print('ERROR: Provided number of samples in samples_coords and samples_covs are inconsistent')
            exit()
        if sample_coords.shape[1] != 2:
            print('ERROR: Wrong sample coordinates format')
            exit()
        self.samples_covs = samples_covs
        self.samples_coords = samples_coords
        if self.sample_num != len(target):
            print('ERROR: Provided number of samples in samples_coords and samples_covs are inconsistent')
            exit()
        self.target = target
        self.stds = stds
        self.means = means
        self.mean_sq = mean_sq
        self.std_sq = std_sq
        self.std_sq4 = std_sq4
        self.datatypes = datatypes
        self.lag = lag
        self.draw = draw
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
    
    
    def calcSimi(covs1,covs2,stds,means,mean_sq,std_sq,std_sq4,datatypes,weights=[]):
        simis=[]
        for i in range(len(stds)):
            v1 = covs1[i]
            v2 = covs2[i]
                
            if datatypes[i]==1:
                if v1==v2:
                    v=1
                else:
                    v=0
            elif datatypes[i]==2:
                v=1-sin(radians(abs(0.5*(v1-v2))))
            else:
                #sij=exp(-pow(v1-v2,2)*0.5/std_sq4[i]*(pow(v1,2)+std_sq[i]+mean_sq[i]-2*v1*means[i])) #ipsm, Zhu et al., 2015
                #sji=exp(-pow(v2-v1,2)*0.5/std_sq4[i]*(pow(v2,2)+std_sq[i]+mean_sq[i]-2*v2*means[i])) #ipsm, Zhu et al., 2015
                #v=(sij+sji)/2
                v = exp(pow((v1-v2)/stds[i],2)*log(0.5)) #range rule
            simis.append(v)
        if(len(weights)==0):
            return np.min(simis)
        else:
            return np.mean(np.array(simis)*weights)

    def gaussian_variogram_model(m, d):
        """Gaussian model, m is [psill, range, nugget]"""
        psill = float(m[0])
        range_ = float(m[1])
        nugget = float(m[2])
        return psill * (1.0 - np.exp(-(d**2.0) / (range_ * 4.0 / 7.0) ** 2.0)) + nugget


    def exponential_variogram_model(m, d):
        """Exponential model, m is [psill, range, nugget]"""
        psill = float(m[0])
        range_ = float(m[1])
        nugget = float(m[2])
        return psill * (1.0 - np.exp(-d / (range_ / 3.0))) + nugget

    def gaussian_variogram_model_simi(m, d):
        """Gaussian model, m is [psill, range, nugget]"""
        psill = float(m[0])
        range_ = float(m[1])
        nugget = float(m[2])
        return psill * (np.exp(-(d**2.0) / (range_ * 4.0 / 7.0) ** 2.0)) + nugget


    def exponential_variogram_model_simi(m, d):
        """Exponential model, m is [psill, range, nugget]"""
        psill = float(m[0])
        range_ = float(m[1])
        nugget = float(m[2])
        return psill * (np.exp(-d / (range_ / 3.0))) + nugget

    def _variogram_residuals(params, x, y, variogram_function, weight):
        """Function used in variogram model estimation. Returns residuals between
        calculated variogram and actual data (lags/semivariance).
        Called by _calculate_variogram_model.
        Parameters
        ----------
        params: list or 1D array
            parameters for calculating the model variogram
        x: ndarray
            lags (distances) at which to evaluate the model variogram
        y: ndarray
            experimental semivariances at the specified lags
        variogram_function: callable
            the actual funtion that evaluates the model variogram
        weight: bool
            flag for implementing the crude weighting routine, used in order to
            fit smaller lags better
        Returns
        -------
        resid: 1d array
            residuals, dimension same as y
        """

        # this crude weighting routine can be used to better fit the model
        # variogram to the experimental variogram at smaller lags...
        # the weights are calculated from a logistic function, so weights at small
        # lags are ~1 and weights at the longest lags are ~0;
        # the center of the logistic weighting is hard-coded to be at 70% of the
        # distance from the shortest lag to the largest lag
        if weight:
            drange = np.amax(x) - np.amin(x)
            k = 2.1972 / (0.1 * drange)
            x0 = 0.7 * drange + np.amin(x)
            weights = 1.0 / (1.0 + np.exp(-k * (x0 - x)))
            weights /= np.sum(weights)
            resid = (variogram_function(params, x) - y) * weights
        else:
            resid = variogram_function(params, x) - y

        return resid

    def calcuate_function_simi(lags, semivariance, variogram_model, weight):
        x0 = [
                np.amax(semivariance) - np.amin(semivariance),
                0.25,
                np.amin(semivariance),
            ]
        bnds = (
                [0.0, 0.0, 0.0],
                [np.amax(semivariance),10, np.amax(semivariance)],
            )
        
        if variogram_model == "gaussian":
            variogram_function = gaussian_variogram_model_simi
        else:
            variogram_function = exponential_variogram_model_simi

        # use 'soft' L1-norm minimization in order to buffer against
        # potential outliers (weird/skewed points)
        res = least_squares(
            _variogram_residuals,
            x0,
            bounds=bnds,
            loss="soft_l1",
            args=(lags, semivariance, variogram_function, weight),
        )

        return res.x

    def calcuate_function(lags, semivariance, variogram_model, weight):
        x0 = [
                np.amax(semivariance) - np.amin(semivariance),
                0.25 * np.amax(lags),
                np.amin(semivariance),
            ]
        bnds = (
                [0.0, 0.0, 0.0],
                [10.0 * np.amax(semivariance), np.amax(lags), np.amax(semivariance)],
            )
        if variogram_model == "gaussian":
            variogram_function = gaussian_variogram_model
        else:
            variogram_function = exponential_variogram_model

        # use 'soft' L1-norm minimization in order to buffer against
        # potential outliers (weird/skewed points)
        res = least_squares(
            _variogram_residuals,
            x0,
            bounds=bnds,
            loss="soft_l1",
            args=(lags, semivariance, variogram_function, weight),
        )

        return res.x

    def calcID(samples_covs,sample_coords,target,stds,means,mean_sq,std_sq,std_sq4,datatypes,lag=50,draw =False):
        sample_num = len(target)
        simis = []
        varios = []
        dists = []
        for i in range(sample_num):
            for j in range(sample_num):
                if i==j:
                    continue
                simis.append( calcSimi(samples_covs[i],samples_covs[j],stds,means,mean_sq,std_sq,std_sq4,datatypes))
                varios.append((target[i]-target[j])**2)
                dists.append(np.sqrt(np.power(sample_coords[i,0]-sample_coords[j,0],2)+
                                    np.power(sample_coords[i,1]-sample_coords[j,1],2)))
        simis = np.array(simis)
        varios = np.array(varios)
        dists = np.array(dists)
        #sei = np.sum(varios*simis)/np.sum(simis)/np.mean(varios)
        
        r = np.max(target)-np.min(target)

        # eid
        sep = 1.0/lag
        start = 0
        end = 0+sep
        simi_stats=[]
        vario_simi_stats=[]
        for i in range(lag):
            selected = np.logical_and(np.array(simis)>start ,np.array(simis)<end)
            simi_select = np.array(simis)[selected]
            if len(simi_select)>0:
                simi_stats.append(np.mean(simi_select))
                vario_simi_stats.append(np.mean(np.array(varios)[selected]))
            start = start+sep
            end = end+sep
        simi_stats = np.array(simi_stats)[np.array(vario_simi_stats)>0]
        vario_simi_stats = np.array(vario_simi_stats)[np.array(vario_simi_stats)>0]
        model = "gaussian"
        #if id_num[0] in [61,187,276,289,351,355,384,417]:
        #    model = "gaussian"
        p = calcuate_function_simi(simi_stats, vario_simi_stats, model, False)
        simi_nugget = p[2]
        eid = sqrt(simi_nugget)/r
        if draw is True:
            fig, (ax1, ax2) = plt.subplots(1, 2)
            ax1.plot(simi_stats, vario_simi_stats, "r*")
            if model == "exp":
                ax1.plot(
                    np.linspace(0,1,100),
                    exponential_variogram_model_simi(p, np.linspace(0,1,100)),
                    "k-",
                )
            else:
                ax1.plot(
                    np.linspace(0,1,100),
                    gaussian_variogram_model_simi(p, np.linspace(0,1,100)),
                    "k-",
                )

        # did
        sep = (np.max(dists)-np.min(dists))/(lag+1)
        start = np.min(dists)
        end = start+sep
        dist_stats=[]
        vario_dist_stats=[]
        for i in range(lag):
            selected = np.logical_and(np.array(dists)>start ,np.array(dists)<end)
            dist_select = np.array(dists)[selected]
            if len(dist_select)>0:
                dist_stats.append(np.mean(dist_select))
                vario_dist_stats.append(np.mean(np.array(varios)[selected]))
            start = start+sep
            end = end+sep
        dist_stats = np.array(dist_stats)[np.array(vario_dist_stats)>0]
        vario_dist_stats = np.array(vario_dist_stats)[np.array(vario_dist_stats)>0]
        model = "gaussian"
        p = calcuate_function(dist_stats, vario_dist_stats, model, False)
        dist_nugget = p[2]
        did = sqrt(dist_nugget)/r
        if draw is True:
            ax2.plot(dist_stats, vario_dist_stats, "r*")
            if model == "exp":
                ax2.plot(
                    np.linspace(0,np.max(dist_stats),100),
                    exponential_variogram_model(p, np.linspace(0,np.max(dist_stats),100)),
                    "k-",
                )
            else:
                ax2.plot(
                    np.linspace(0,np.max(dist_stats),100),
                    gaussian_variogram_model(p, np.linspace(0,np.max(dist_stats),100)),
                    "k-",
                )
            plt.show()

        return did,eid