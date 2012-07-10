% Script to generate time differences of FITS images
% pep/20Jun12
%fitsfolder = '/home/aartfaac/Peeyush/imaging/build/data/output_5sb_try3_1415-1420/59756469/fits/'
fitsfolder = '/Users/peeyush/Documents/Oxford-June12/output_5sb_try3_1415-1420/59951782/fits/';

[~, lsout] = dos(['ls -1 ' fitsfolder '*.fits']);
datafiles = textscan(lsout, '%s\n');
nfiles = length(datafiles{1});
disp (['Found ' num2str(nfiles) ' Files']);

previmg = fitsread (datafiles{1}{1});
for ind = 2:nfiles
	info = fitsinfo (datafiles {1}{ind});
	disp (['File: ' info.Filename ', Size ' num2str(info.PrimaryData.Size)]);
	currimg = fitsread (datafiles{1}{ind});
	imdiff = currimg - previmg; 
	previmg = currimg;
	
	subplot (1,2,1);
	imagesc (currimg);
	colorbar ();
	subplot (1,2,2);
	imagesc (imdiff);
	colorbar ();
	pause (1);
end
	


