function [radecskymap, lmskymap, vispad] = fft_imager_sjw_radec(acc, u, v, duv, Nuv, uvsize, t_obs, freq)

% create object for interpolation
vis = zeros(Nuv);
%W = zeros(Nuv);
for idx = 1:length(u(:)) 
    ampl = abs(acc(idx));
    phasor = acc(idx) / ampl;
    uidx = u(idx) / duv + Nuv / 2;
    uidxl = floor(uidx);
    uidxh = ceil(uidx);
    dul = abs(uidx - uidxl);
    duh = abs(uidx - uidxh);
    sul = duh * ampl;
    suh = dul * ampl;
    
    vidx = v(idx) / duv + Nuv / 2;
    vidxl = floor(vidx);
    vidxh = ceil(vidx);
    dvl = abs(vidx - vidxl);
    dvh = abs(vidx - vidxh);
    sull = dvh * sul;
    sulh = dvl * sul;
    suhl = dvh * suh;
    suhh = dvl * suh;
    
    vis(uidxl, vidxl) = vis(uidxl, vidxl) + sull * phasor;
    vis(uidxl, vidxh) = vis(uidxl, vidxh) + sulh * phasor;
    vis(uidxh, vidxl) = vis(uidxh, vidxl) + suhl * phasor;
    vis(uidxh, vidxh) = vis(uidxh, vidxh) + suhh * phasor;
    
    %W(uidx, vidx) = W(uidx, vidx) + 1;
end

% zero padding to desired (u,v)-size
N = size(vis, 1);
N1 = floor((uvsize - N) / 2);
N2 = ceil((uvsize + 1 - N) / 2) - 1;
vispad = [zeros(N1, uvsize); ...
          zeros(N, N1), vis, zeros(N, N2); ...
          zeros(N2, uvsize)];
vispad(~isfinite(vispad)) = 0;

% compute image
% ac = zeros(size(vispad));
% ac(256, 256) = 100;
% vispad = vispad + ac;
vispad = conj(flipud(fliplr(fftshift(vispad))));
skymap = fftshift(fft2(vispad));

% Create l,m axis corresponding to choices of duv
dl = (299792458/(freq * uvsize * duv)); % dimensionless, in dir. cos. units
lmax = dl * uvsize / 2;
l = [-lmax:dl:lmax-1e-3];
m = l;  % Identical resolution and extent along m-axis

mask = zeros (length(l));
mask(meshgrid(l).^2 + meshgrid(l).'.^2 < 1) = 1;
lmskymap = single (abs(skymap) .* mask);
disp (['-->Max/min from matlab: ' num2str(max(max(lmskymap))) ' ' num2str(min(min(lmskymap)))]);

% Section converting image from local coordinates to RA/DEC coordinates
alpha = zeros (uvsize, 1);
delta = zeros (uvsize, 1);
t_obs  = t_obs/86400 + 2400000.5; % Convert to MJD day units
[alpha, delta] = lmtoradec (l, l, t_obs);
sel = ~isnan (alpha(:));

radecimage = TriScatteredInterp (alpha (sel), delta (sel), abs(skymap (sel)));
% Create the regularly sampled RA/dec plane
[ragrid, decgrid] = meshgrid (linspace (0,2*pi, uvsize), linspace (-pi/2,pi/2, uvsize));
    
% generate samples from model skyimage
radecskymap = radecimage (ragrid, decgrid);

