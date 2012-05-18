% Program to convert an image generated in l,m local coordinates to RA/DEC coordinates.
% pep/27Apr12
function [alpha, delta, radecimg] = lmtoradecimage (skymap, l, m, t_obs)
	if (isempty (skymap) || isempty (l) || isempty(m) || isempty (t_obs))
		disp ('### Error! empty input parameters found...');
		return;
	end
	uvpad = length (skymap);
	alpha = zeros (uvpad, 1);
	delta = alpha;  % NOTE: Assuming square images.
	[alpha, delta] = lmtoradec (l, m, t_obs);	
	sel = ~isnan (alpha(:));

	% Then interpolate the image to create a model surface
	radecimage = TriScatteredInterp (alpha (sel), delta (sel), abs(skymap (sel)));

	% Create the regularly sampled RA/dec plane	
	[ragrid, decgrid] = meshgrid (linspace (0,2*pi, uvpad), linspace (-pi/2,pi/2, uvpad));
    
        % generate samples from model skyimage
	radecimg = zeros (uvpad, uvpad);
        radecimg(:,:) = radecimage (ragrid, decgrid);

