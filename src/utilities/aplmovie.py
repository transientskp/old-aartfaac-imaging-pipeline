##!/opt/local/bin/ipython-2.7

#from pylab import *;

#import matplotlib;
#matplotlib.use ('Agg');
#
import aplpy;
import glob;
import os;
import re;

os.system ('mkdir png');
for imagename in glob.glob('fits/*.fits'):
	print 'Working on file : ', imagename;
	m = re.search ('_(.+?).image', imagename);
	#outfile = 'png/'+imagename + '.png';
	ffits = aplpy.FITSFigure(imagename); 
#	ffits.show_colorscale(cmap='gist_heat', vmin='200', stretch='linear');
	ffits.show_colorscale(cmap='hot', vmin=-50, vmax=450,stretch='power', smooth=3, kernel='gauss');
	#ffits.show_colorscale(vmin=300, vmax=450, stretch='linear', smooth=3, kernel='gauss');
	ffits.add_colorbar();
	a = ffits.pixel2world(400, 500);
	if m:
		ffits.add_label(a[0], a[1], 'Time: '+m.group(1), color='Black');
	ffits.tick_labels.hide(); # Hide the x and y tick labels
	ffits.axis_labels.set_font (size=14);
	ffits.axis_labels.set_xtext ('East -- l -- West');
	ffits.axis_labels.set_ytext ('North -- m -- South');
	ffits.frame.set_linewidth(2);
	ffits.frame.set_color('green');
	a = ffits.pixel2world(256, 256);
	ffits.show_circles (a[0], a[1], a[0]);
	ffits.save (imagename+'.png');
	ffits.close ();
