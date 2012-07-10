% Function to resize the uloc/vloc data read from file.
function [uloc_flag, vloc_flag] = gen_flagged_uvloc (uloc, vloc, flagant)
	uloc = reshape (uloc, [288, 288]);
	vloc = reshape (vloc, [288, 288]);
	mask = zeros (size (uloc));
	rem_ants = length (uloc) - length(flagant);
	for ind = 1:length(flagant)
	   mask(flagant(ind), :) = 1; mask(:,flagant(ind)) = 1;
	end
	uloc_flag = reshape(uloc(mask ~= 1), [rem_ants, rem_ants]);
	vloc_flag = reshape(vloc(mask ~= 1), [rem_ants, rem_ants]);
