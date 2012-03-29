% Program to simulate a pelican pipeline, using the pelican_calib ()
% and fft_imager_sjw () functions.
% pep/23Mar12

function pelica_pipesim (fname)
    Nelem = 288; 
    nblines = Nelem * (Nelem + 1)/2; 
    uvflag = eye (288); % Flag only the autocorrelations
    freq = 59951782.2;
    mins2cal = 2;
    
    % FFT Imaging related
    duv = 600/511;                % In meters, grid spacing in fft imager
    Nuv = 512;                    % size of gridded visibility matrix
    uvpad = 512;                  % specifies if any padding needs to be added

    % Local horizon based coordinates of array in ITRF
    load ('poslocal.mat', 'posITRF', 'poslocal'); 

    % Generate uv coordinates in local horizon coordinate system, needed for imaging
    uloc = meshgrid (poslocal(:,1)) - meshgrid (poslocal (:,1)).';
    vloc = meshgrid (poslocal(:,2)) - meshgrid (poslocal (:,2)).';
    % normal to CS002 field (ITRF)
    normal = [0.598753, 0.072099, 0.797682].'; 

    
    fid = fopen (fname, 'rb');
    outfilename = strrep (fname, '.bin', '_pelpipe.mat');
    disp(['Writing cal solutions to file: ' outfilename]);

    t_obs = fread (fid, 1, 'double');
    disp (['Timeslice: ' num2str(t_obs, '%f') ' (MJD secs) for initial estimation']);
    

    % Reading real and imaginary, available as a stream of floats.
    % even floats being real parts, odd floats being imag
    a = fread (fid, 2*nblines, 'float'); % Read one ccm worth
    comp = complex (a(1:2:length(a)), a(2:2:length(a))); % to complex
    % create instantaneous ccm from vector
    acm = triu (ones (Nelem));
    acm (acm == 1) = comp;
    acc = acm + acm' - diag(diag(acm));
    
    t_obs1 = fread (fid, 1, 'double');
    tslices2cal = floor ((mins2cal*60)/(t_obs1 - t_obs)); % recalibrate every 2 mins 
    disp (['Time resolution of observations: ' num2str(t_obs1 - t_obs) ' secs']);
    disp (['Calibrating every ' num2str(tslices2cal) ' timeslices']);
    
    % Convert back to Julian day, as desired by track_statcal. NOTE:
    % JulianDay () should not be called now!
    t_obs  = t_obs/86400 + 2400000.5; % Convert to day units
    t_obs1 = t_obs1/86400 + 2400000.5;
   
    % Go to the beginning of the file, read out one timeslice
    fseek (fid, 0, -1);
    t_obs = fread (fid, 1, 'double');
    if (isempty (t_obs))
       return;
    end
    a = fread (fid, 2*nblines, 'float'); % Read one ccm worth
    if (isempty(a))
       return;
    end
     

    %% calibrate using source positions from catalog and a baseline
    % restriction of 10 wavelengths or 60 meters (whichever is smaller)
    % This results in estimated source fluxes of the model sources
    [calvis, gainsol, sigmas, sigman, good] = pelican_calib (acc, t_obs, freq, uvflag);
    [calmap calvis] = fft_imager_sjw (calvis (:), uloc(:), vloc(:), duv, Nuv, uvpad);              
    t_obs_store (1) = t_obs;
    cal_store (:,1) = gainsol;
    sigmas_store = {};
    sigmas_store{1} = sigmas;
    sigman_store(:,:,1) = sigman;
    image_store (:,:,1) = calmap; 
    
    t = 1;
    calrun = 1;
    stop_processing = 0 ;
    %%
   try
        while (feof (fid) ~= 1 & stop_processing == 0)
        % for i=1:10

            t = t + 1;

            t_obs = fread (fid, 1, 'double');
            if (isempty (t_obs))
                break;
            end
            disp (['Timeslice: ' num2str(t) ' (' num2str(t_obs, '%f') ')']);

            % Reading real and imaginary, available as a stream of floats.
            % even floats being real parts, odd floats being imag
            a = fread (fid, 2*nblines, 'float'); % Read one ccm worth
            if (isempty(a))
                break;
            end
            comp = complex (a(1:2:length(a)), a(2:2:length(a))); % to complex
            % create instantaneous ccm from vector
            acm = triu (ones (Nelem));
            acm (acm == 1) = comp;
            acc = acm + acm' - diag(diag(acm));
            
            % Convert back to Julian day, as desired by track_statcal. NOTE:
            % JulianDay () should not be called now!
            calrun = calrun + 1;            
            t_obs_store (calrun) = t_obs;           
            t_obs = t_obs/86400 + 2400000.5;
            
            % Calibrate this timeslice
            [calvis, gainsol, sigmas, sigman, good] = pelican_calib (acc, t_obs, freq, uvflag);
            cal_store (:,calrun) = gainsol;
            sigmas_store {calrun} = sigmas;
            sigman_store (:,:,calrun) = sigman;            

            % Image this data
            [calmap calvis] = fft_imager_sjw (calvis (:), uloc(:), vloc(:), duv, Nuv, uvpad);  
            image_store (:,:,calrun) = calmap;
            
            for t_ind = 1:tslices2cal
                t_obs = fread (fid, 1, 'double');
                a = fread (fid, 2*nblines, 'float'); % Read one ccm worth
            end    
        end
    catch err
        disp ('Error encountered! Saving variables to disk..');
        save (outfilename, 'cal_store', 'sigmas_store', 'sigman_store', 't_obs_store', 'image_store');
        rethrow (err);
    end

    disp ('Calibration check completed. Storing variables to file');
    save (outfilename, 'cal_store', 'sigmas_store', 'sigman_store', 't_obs_store', 'image_store');
