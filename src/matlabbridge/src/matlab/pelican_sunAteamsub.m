% Program to implement Sun and A-team subtraction, after carrying out a 
% StefCal stage to completion. The function operates on a single timeslice of a single channel
% and basically packages aartfaac_demo_stefcal.m to be called from a pelican pipeline.
% pep/14Apr2012

function [thsrc_cat, phisrc_cat, thsrc_wsf, phisrc_wsf, suncomps, calvis, gainsol, sigmas, sigman, good] = pelican_sunAteamsub (acc, t_obs, freq, uvflag, flagant, debug, ptSun)
% function [calvis, gainsol, sigmas, sigman, good] = pelican_sunAteamsub (acc, t_obs, freq, uvflag, debug)
% Arguments:
% t_obs : Time in MJD secs, as extracted from a MS
% acc   : Filled, square ACM of size NantxNant complex numbers, where Nant=total unflagged antennas
% freq  : The frequency of the observation in Hz.
% uvflag: Mask for flagged visibilities, which are ignored 
% flagant: Numerical list of antennas which are flagged, leading to a reshaping of the ACM.
% debug : Level of debug messages: 
%         0 ==> Only very important messages
%         1 ==> More detail
%         2 ==> Even more detail
% ptSun : 0 ==> Carry out a Sparse reconstruction of the Solar model.
%         1 ==> Model the Sun as a point source.

% Return values:
% thsrc_cat: Elevation (rad) of catalog sources at t_obs, in an Equatorial coord. sys, derived from ITRF coordinates.
% phisrc_cat: Azimuth (rad) of catalog sources at t_obs.
% thsrc_wsf: Elevation (rad) of a catalog source, as determined using WSF on the data.
% azisrc_wsf: Azimuth (rad) of a catalog source, as determined using WSF on the data.
% calvis : Calibrated visibilities, ready for imaging
% gainsol: Estimated per-element complex gains
% sigmas : Estimated flux of model sources
% sigman : Estimated system temprature per baseline
% good   : Boolean encoding goodness of calibration solution

    % Load once only
    persistent first_call;
    persistent srclist3CR;
    persistent posITRF;
    persistent poslocal;

    if (isempty (first_call))
        disp ('   --- Non Tracking Stef. calibration to convergence ---   ');
        disp ('Loading 3CR catalog and local antenna positions.');
    	load srclist3CR
    	load ('poslocal.mat', 'posITRF', 'poslocal'); 
        first_call = 0;
    end

    % disp ('---> Loading acc.mat');
    % load ('acc.mat');
    C = 299792458;               % speed of light, m/s
    lon = 6.869837540;           % longitude of CS002 in degrees
    lat = 52.915122495;          % latitude of CS002 in degrees
    restriction = 4;             % Avoid uv values with <10 wavelengths length
    maxrestriction = 60;         % Avoid uv values with >60m length
    Nelem = 288;                 % NOTE: Hardcoded!
    nblines = Nelem * (Nelem + 1)/2; 
    normal = [0.598753, 0.072099, 0.797682].';
    srcsel =  [324, 283, 88, 179, 0]; % A team from 3CR catalog
    good = 1;                    % Optimistic approach!
	
    if (isempty (acc) | isempty (t_obs))
        disp ('ACM or time missing!');   
        return;
    end
    disp (['t_obs :' num2str(t_obs) ' Freq: ' num2str(freq)]);

    % if (~isempty (flagant)) % Need to reshape the array
    if (length(flagant) ~= 0) % Need to reshape the array
	disp (['Flagant' num2str(flagant)]);
	antmask = zeros (size (acc));
	posmask = zeros (size (posITRF));
	rem_ants = length(acc) - length(flagant);
	for ind = 1:length(flagant)
		antmask (flagant(ind), :) = 1; antmask (:,flagant(ind)) = 1;
		posmask (flagant(ind), :) = 1;
	end
	acc = reshape (acc(antmask ~= 1), [rem_ants, rem_ants]);
	posITRF_fl = reshape (posITRF(posmask ~=1), [rem_ants, 3]);
	uvflag  = reshape (uvflag(antmask ~= 1), [rem_ants, rem_ants]);
	disp (['NOTE: ACM resized after flagging to ' num2str(size (acc))]);
	%disp (['Size of uvflag, antmask' num2str(size(uvflag), size (antmask))]);
    else
      posITRF_fl = posITRF;
    end
    t_obs  = t_obs/86400 + 2400000.5; % Convert to MJD day units
    catalog = srclist3CR;
    
    % whitening of the array covariance matrix (required for DOA estimation)
    % tic
    acc = acc ./ sqrt(diag(acc) * diag(acc).');
    % calibrate using source positions from 3C catalog and a baseline
    % restriction of 10 wavelengths or 60 meters (whichever is smaller)
    [cal1, sigmas1, Sigman1] = statcal_stefcal (acc, t_obs, freq, posITRF_fl, srcsel, normal, 10, 60, uvflag, debug);
    % [cal1, sigmas1, Sigman1] = statcal_vlss(acc, t_obs, freq, posITRF, srcsel, normal, 10, maxrestriction, eye(Nelem), catalog);
    if (debug > 1)
      disp('First round of antenna calibration completed, sigmas1:'); disp (sigmas1');
    end
    % toc

    
    % Actual source positions may differ from the known positions due to
    % ionospheric image distortions, so we use a high-resolution DOA estimation
    % technique (weighted subspace fitting, WSF) to estimate source positions
    % relative to the position of Cas A.
    % We only estimate source positions for sources with an apparent flux
    % larger than 1% of the apparent flux of Cas A.
    % tic
    % use source positions from 3C catalog as initial estimate
    % load srclist3CR
    if (sum(srcsel == 0) ~= 0)
        % [raSun, decSun] = SunRaDec(JulianDay(tobs));                % J2000
        [raSun, decSun] = SunRaDec(t_obs);                % J2000
        rasrc = [srclist3CR(srcsel(srcsel ~= 0)).alpha, raSun].';   % B1950
        decsrc = [srclist3CR(srcsel(srcsel ~= 0)).delta, decSun].'; % B1950
        epoch = [true(length(srcsel(srcsel ~= 0)), 1); false];
        % disp (['Sun included, len of epoch ' num2str(length(epoch))]);
    else
        rasrc = [srclist3CR(srcsel(srcsel ~= 0)).alpha];  % B1950
        decsrc = [srclist3CR(srcsel(srcsel ~= 0)).delta]; % B1950
        epoch = true(length(srcsel), 1);
        % disp (['Sun NOT included, len of epoch ' num2str(length(epoch))]);
    end
    % srcpos0 = radectoITRF(rasrc(sel), decsrc(sel), epoch, JulianDay(tobs));
    % srcpos0 = radectoITRF(rasrc(sel), decsrc(sel), epoch(sel), t_obs);
    srcpos0 = radectoITRF(rasrc, decsrc, epoch, t_obs); % Postions of all srcs
    up = srcpos0 * normal > 0;
    sel = sigmas1 > 0.01 & up'; % Srcs with enough power, and above the horizon
    nsrc = sum(sel);
    if (debug > 0)
       disp (['WSF: Working with ' num2str(nsrc) ' selected sources, t_obs(MJD): ' num2str(t_obs)]);
    end
    disp(['Srcs from srclist above horizon: ' num2str(srcsel(up))]);
    thsrc0 = asin(srcpos0(sel, 3));
    phisrc0 = atan2(srcpos0(sel, 2), srcpos0(sel, 1));
    
    % find source positions using WSF algorithm
    % whitening of the array covariance matrix (required for DOA estimation)
    % acc = acc ./ sqrt(diag(acc) * diag(acc).');
    [v, d] = eig(acc);
    [d, order] = sort(diag(abs(d)), 'descend');
    v = v(:, order);
    Wopt = (diag(d(1:nsrc)) - mean(diag(squeeze(Sigman1))) * eye(nsrc))^2 / diag(d(1:nsrc));
    Es = v(:, 1:nsrc);
    EsWEs = Es * Wopt * Es';
    theta = fminsearch(@(x) WSFcostfunITRF(x, EsWEs, diag(conj(1 ./ cal1)), freq, posITRF_fl), [phisrc0; thsrc0]);
    phisrchat = zeros (length(srcsel),1); phisrchat(sel) = theta(1:nsrc);
    thsrchat  = zeros (length(srcsel),1); thsrchat (sel) = theta(nsrc+1:end);
    % srcposhat = [cos(phisrchat) .* cos(thsrchat), sin(phisrchat) .* cos(thsrchat), sin(thsrchat)];
    srcposhat = [cos(phisrchat(sel)) .* cos(thsrchat(sel)), sin(phisrchat(sel)) .* cos(thsrchat(sel)), sin(thsrchat(sel))];
    if (debug > 0)
      disp('Position estimation using WSF done: ');
    end 
    if (debug > 1)
     disp ('Catalog positions: '); disp ([thsrc0 phisrc0]');
     disp ('WSF     positions: '); disp ([thsrchat phisrchat]');
    end
    thsrc_cat  = zeros (length(srcsel), 1); thsrc_cat (sel) = thsrc0;
    phisrc_cat = zeros (length(srcsel), 1); phisrc_cat (sel) = phisrc0;
    thsrc_wsf = thsrchat;
    phisrc_wsf = phisrchat;
    % toc
    
    % use updated source positions to improve calibration
    % tic
    A = exp(-(2 * pi * 1i * freq / C) * (posITRF_fl * srcposhat.'));
    [ghat, sigmas2, Sigman2] = cal_ext_stefcal(acc, A, sigmas1(sel), squeeze(abs(Sigman1)) > 0, debug);
    cal2 = (1 ./ ghat)';
    % calibrate visibilities removing extended emission and system noise
    acccal = (cal2' * cal2) .* (acc - squeeze(Sigman2));
    % disp('Calibration factors updated');
    % toc
    
    %% A-team subtraction
    % We start by removing the A-team sources. Since the Sun is a barely
    % resolved source given the AARTFAAC resolution of 0.8 degrees at 60 MHz,
    % it requires separate treatment.
    % The difference between the actual positions as estimated by WSF and the
    % positions in the catalog is sufficiently large to make subtraction based
    % on catalog positions fail, we use the source positions estimated above.
    % reduce srcsel to source that appear brighter than 1% if Cas A, note that
    % 0 is used to denote the Sun
    % tic
    visibleAteamsun = srcsel(sel); % Actual source indices into srclist3CR, don't use directly!
    % construct model visibilities for A-team sources, choosing Point source model of the Sun, 
    % if required.
    if (ptSun == 1)
       disp ('Carrying out Point source subtraction of the Sun');
       A = exp(-(2 * pi * 1i * freq / C) * (posITRF_fl * srcposhat.')); % sel holds the indices of all relevant sources
       RAteam = A * diag(sigmas2) * A';
    else
       A = exp(-(2 * pi * 1i * freq / C) * (posITRF_fl * srcposhat(visibleAteamsun ~= 0, :).'));
       RAteam = A * diag(sigmas2(visibleAteamsun ~= 0)) * A';
    end 
    % subtract A-team
    accsubAteam = acccal - RAteam;
    accsubSunS = accsubAteam;
    if (debug > 0)
      disp('A-team subtracted');
    end 
    % toc


    if (ptSun == 0)
      %% Sun subtraction
      % The Sun is modeled using a 3-sparse grid of pixels around the estimated
      % position of the Sun. High resolution imaging using the MVDR beamformer to
      % obtain a model of the Sun in the image domain proved unsuccessful.
      % Finding the 3-sparse solution using an exhaustive search it the most
      % computationally demanding part of this script. Hopefully Arash can
      % improve on this using convex optimization.
      % tic
      phi0sun = atan2(srcposhat(visibleAteamsun == 0, 2), srcposhat(visibleAteamsun == 0, 1));
      th0sun = asin(srcposhat(visibleAteamsun == 0, 3));
      % define grid of pixels
      delta = 0.01;
      res = 2 * delta / 8;
      [phisun, thsun] = meshgrid(phi0sun-delta:res:phi0sun+delta, th0sun-delta:res:th0sun+delta);
      phith_dist = sqrt((phisun(:) - phi0sun).^2 + (thsun(:) - th0sun).^2);
      phisun = phisun(phith_dist < delta);
      thsun = thsun(phith_dist < delta);
      sunpos = [cos(phisun) .* cos(thsun), sin(phisun) .* cos(thsun), sin(thsun)];
      
      % solar component estimation using sparse reconstruction
      A = exp(-(2 * pi * 1i * freq / C) * (posITRF_fl * sunpos.'));
      A = double (A);
      accsubAteam = double (accsubAteam);
      fluxSun = 1.20 * sigmas2(visibleAteamsun == 0);
      cvx_begin
          variable sigmaSun(size(A, 2));
          minimize norm(accsubAteam(:) - khatrirao(conj(A), A) * sigmaSun, 2);
          subject to
          norm(sigmaSun, 1) <= fluxSun;
      cvx_end
      
      % iterate, increasing resolution
      sigmaSun(sigmaSun < 1e-3) = 0;
      phi0sun = phisun(sigmaSun ~= 0);
      th0sun = thsun(sigmaSun ~= 0);
      res2 = res / 3;
      [phigrid, thgrid] = meshgrid(-res2:res2:res2);
      phisun2 = kron(phi0sun, ones(9, 1)) + kron(ones(length(phi0sun), 1), phigrid(:));
      thsun2 = kron(th0sun, ones(9, 1)) + kron(ones(length(th0sun), 1), thgrid(:));
      sunpos2 = [cos(phisun2) .* cos(thsun2), sin(phisun2) .* cos(thsun2), sin(thsun2)];
      A = exp(-(2 * pi * 1i * freq / C) * (posITRF_fl * sunpos2.'));
      fluxSun = 1.20 * sigmas2(visibleAteamsun == 0);
      cvx_begin
          variable sigmaSun(size(A, 2));
          minimize norm(accsubAteam(:) - khatrirao(conj(A), A) * sigmaSun, 2);
          subject to
          norm(sigmaSun, 1) <= fluxSun;
      cvx_end
      
      % try subtraction
      sigmaSun(sigmaSun < 1e-3) = 0;
      RSunS = A * diag(sigmaSun) * A';
      accsubSunS = acccal - RAteam - RSunS;
      if (debug > 0)
        disp('Sun subtracted using Sparse reconstruction.');
      end
      %toc

      suncomps = zeros (size (sigmaSun, 1), 3); % Storing both fluxes and positions as az/ele
      suncomps (:, 1) = sigmaSun;
      suncomps (:, 2) = atan2 (sunpos2 (:,2), sunpos2 (:, 1));  % Azimuth
      suncomps (:, 3) = acos (sunpos2 (:,3));
    else
      suncomps = zeros (1, 3);  % Placeholder for when a pt. src sun subtraction
                                % is adequate.
    end  % if (ptSun == 1) ... 


    % calvis = accsubAteam;
    calvis = single(accsubSunS);
    gainsol = cal2;
    sigmas(sel) = sigmas2;
    sigman = Sigman2;
    % disp (['Class of calvis/gainsol/sigmas/sigman' class(calvis) class(gainsol) class(sigmas) class(sigman)]);

    % Quality of calibration solutions, as determined by the phase solutions
%    flag_ant_stations = zeros (1,6);
%    for ant = 1:length(flagant)
%        % Which station does current flagged antenna belong
%	flag_station = int32(flagant(ant)/48) + 1; 
%	flag_ant_stations (flag_station) = flag_ant_stations(flag_station) + 1;
%    end 
    % disp (['Flag_ant_stations ' num2str(flag_ant_stations)]);

%    ant_ind = 1;
%    for station=1:6
%      ph_var (station) = var (angle(gainsol (1+(station-1)*48:station*48)));
%%      ph_var (station) = var (angle(gainsol (ant_ind:ant_ind+47-flag_ant_stations(station))));
%%      ant_ind = ant_ind + 47 -flag_ant_stations(station) + 1;
%    end
%    if sum(ph_var > 1) ~= 0 
%      good = 0;
%      disp ('NOTE: Bad calib solu:');
%      disp (ph_var');
%    else
%      good = 1;
%    end 
