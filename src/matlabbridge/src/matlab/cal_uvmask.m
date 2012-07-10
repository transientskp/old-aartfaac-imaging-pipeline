% Script to apply a uvdistance based mask on passed visibilities
function accmask = cal_uvmask(restriction, freq)
	load ('poslocal.mat', 'poslocal');
	uloc = meshgrid (poslocal(:,1)) - meshgrid (poslocal(:,1)).';
	vloc = meshgrid (poslocal(:,2)) - meshgrid (poslocal(:,2)).';
	wloc = meshgrid (poslocal(:,3)) - meshgrid (poslocal(:,3)).';
	uvw = [uloc(:), vloc(:), wloc(:)];
	normal = [0.598753, 0.072099, 0.797682].';
	uvdist = sqrt (sum(uvw.^2,2) - (uvw * normal).^2);
	accmask = reshape (uvdist, [288, 288]) < restriction*(299792458/freq);
