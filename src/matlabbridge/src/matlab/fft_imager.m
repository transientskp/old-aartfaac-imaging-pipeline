function [lmskymap, vispad] = fft_imager(acc, u, v, duv, Nuv, uvsize)

re=real(acc(:)); im = imag(acc(:));
for i=1:length(acc)
   disp ([num2str(re(i)) ' ' num2str(im(i))]);
end
% create object for interpolation
vis = zeros(Nuv);
%W = zeros(Nuv);
for idx = 1:length(u(:)) 
    ampl = abs(acc(idx));
    phasor = acc(idx) / ampl;
    uidx = u(idx) / duv + Nuv/2;
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
skymap = fftshift(fft2(vispad));
lfft = linspace (-1, 1, length (skymap));
mask = zeros (length(lfft));
mask(meshgrid(lfft).^2 + meshgrid(lfft).'.^2 < 1) = 1;
lmskymap = single (abs(skymap) .* mask);
disp (['-->Max/min from matlab: ' num2str(max(max(lmskymap))) ' ' num2str(min(min(lmskymap)))]);
