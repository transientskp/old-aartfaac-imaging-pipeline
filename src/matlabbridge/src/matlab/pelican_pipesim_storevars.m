% Program to simulate a pelican pipeline, using the pelican_calib ()
% and fft_imager_sjw () functions.
% pep/23Mar12

function pelican_pipesim (fname)
   %%
      Nelem = 288; 
    nblines = Nelem * (Nelem + 1)/2; 
    uvflag = eye (288); % Flag only the autocorrelations
    % freq = 59951782.2;
    freq = 59756469;
    mins2cal = 2;
    debuglev = 2;
    skiprecs = 0;
    recoffset = 10;

    % FFT Imaging related
    % duv = 600/511;                % In meters, grid spacing in fft imager
    duv = 2;
    Nuv = 1000;                    % size of gridded visibility matrix
    uvpad = 1024;                  % specifies if any padding needs to be added

    % Local horizon based coordinates of array in ITRF
    load ('poslocal.mat', 'posITRF', 'poslocal'); 

    % Generate uv coordinates in local horizon coordinate system, needed for imaging
    uloc = meshgrid (poslocal(:,1)) - meshgrid (poslocal (:,1)).';
    vloc = meshgrid (poslocal(:,2)) - meshgrid (poslocal (:,2)).';
    % normal to CS002 field (ITRF)
    normal = [0.598753, 0.072099, 0.797682].'; 

    
    fid = fopen (fname, 'rb');
    % fseek (fid, 0, -1);
    for recs = 1:recoffset
      t_obs = fread (fid, 1, 'double');
      a = fread (fid, 2*nblines, 'float'); % Read one ccm worth
    end    

   t = 1;
   try
        while (feof (fid) ~= 1 )
        % for i=1:5

            t = t + 1;

            t_obs = fread (fid, 1, 'double');
            if (isempty (t_obs))
                break;
            end
            disp ('---->');
            outfilename = sprintf ('%d.mat',int64(t_obs));
            disp (['Timeslice: ' num2str(t) ' (' num2str(t_obs, '%f') '), to file: ' outfilename]);

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
%            calrun = calrun + 1;            
%            t_obs_store (calrun) = t_obs;           
            % t_obs = t_obs/86400 + 2400000.5;
            
            % Calibrate this timeslice
            % [calvis, gainsol, sigmas, sigman, good] = pelican_calib (acc, t_obs, freq, uvflag);
            [thsrc_cat, phisrc_cat, thsrc_wsf, phisrc_wsf, calvis, gainsol, sigmas, sigman, good] = pelican_sunAteamsub (acc, t_obs, freq, uvflag, posITRF);
            % Image this data
            % [calmap, calvis, l, m] = fft_imager_sjw (calvis (:), uloc(:), vloc(:), duv, Nuv, uvpad, freq);  
            [calmap, ~] = fft_imager (calvis, uloc, vloc, duv, Nuv, uvpad);

            
            for recs = 1:skiprecs
                t_obs = fread (fid, 1, 'double');
                a = fread (fid, 2*nblines, 'float'); % Read one ccm worth
            end    
            save (outfilename, 'gainsol', 'sigmas', 'sigman', 't_obs', 'calmap', 'good', 'thsrc_cat', 'phisrc_cat', 'thsrc_wsf', 'phisrc_wsf');
        end
    catch err
        disp ('Error encountered! Saving variables to disk..');
        % save (outfilename, 'gainsol', 'sigmas', 'sigman', 't_obs', 'calmap', 'good');
        rethrow (err);
    end

