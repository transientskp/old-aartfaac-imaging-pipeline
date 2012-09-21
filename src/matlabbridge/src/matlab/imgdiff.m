% Program to generate the difference between two images (generated via 
% fft_imager).
% pep/18Jul12
% Arguments: 
%    t_obs    : Time of this data set, in MJD secs
%    freq     : Frequency of data set, in Hz
%    img_curr : Image for current time/freq slice
%    img_prev : Image for previous time/freq slice
% Return:
%    diff     : Pixel by pixel image difference

function [diff] = imgdiff (img_curr, img_prev, t_obs, freq)
	diff = img_curr - img_prev;
	disp ('imgdiff: Setting prev image to current image');
	disp (['Time/Freq' num2str(t_obs) ' '  num2str(freq)]);
	img_prev = img_curr;
	fname = sprintf ('%8.0f_%10.0f_diff.mat', freq, t_obs);
	save (fname, 't_obs', 'freq', 'diff');
