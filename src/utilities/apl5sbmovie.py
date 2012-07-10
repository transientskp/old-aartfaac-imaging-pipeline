##!/opt/local/bin/ipython-2.7

#from pylab import *;

#import matplotlib;
#matplotlib.use ('Agg');
#
import aplpy;
import matplotlib.pyplot as mpl;
import glob;
import os;
import re;

print 'Generating images for all subbands simultaneously.'
sb4 = glob.glob ('60537719/fits/*.fits');
sb3 = glob.glob ('60342407/fits/*.fits');
sb2 = glob.glob ('60147094/fits/*.fits');        
sb1 = glob.glob ('59951782/fits/*.fits');



# Setup montage
fig = mpl.figure(figsize=(9,7))
dx = 0.35; dy = 0.43; 
x1 = 0.1; x2 = 0.55; y1 = 0.1; y2 = 0.565;
label_x = 390; label_y = 490;
# fig = mpl.figure()

usable_files = min(len(sb1), len(sb2), len (sb3), len(sb4));
print 'Found ', usable_files, ' common files';
os.system ('mkdir comb_frames/');
for ind in range (1, usable_files):
#for imagename in glob.glob('*.fits'):
	m = re.search ('_(.+?).image', sb1[ind]);
	outfile = 'comb_frames/'+m.group(1)+'.png';
	print 'Working on time: ', m.group(1), ', combined plot to', outfile;
	sb1f1 = aplpy.FITSFigure(sb1[ind], figure=fig, subplot=[x1,y1,dx,dy])
#	ffits.show_colorscale(cmap='gist_heat', vmin='200', stretch='linear');
	sb1f1.show_colorscale(cmap='hot', vmin=10, vmax=450, stretch='power', smooth=3, kernel='gauss');
	#ffits.show_colorscale(vmin=300, vmax=450, stretch='linear', smooth=3, kernel='gauss');
	sb1f1.add_colorbar();
	a = sb1f1.pixel2world(label_x, label_y);
	b = sb1f1.pixel2world(label_x-250, label_y);
	if m:
		sb1f1.add_label(a[0], a[1], 'F: 59951782', color='white');
		sb1f1.add_label(b[0], b[1], 'T: '+m.group(1), color='white');
	sb1f1.tick_labels.hide(); # Hide the x and y tick labels
	sb1f1.axis_labels.set_xtext ('East -- l -- West');
	sb1f1.axis_labels.set_ytext ('North -- m -- South');
	sb1f1.frame.set_linewidth(2);
	sb1f1.frame.set_color('green');
	sb1f1.close ();

	sb2f2 = aplpy.FITSFigure(sb1[ind], figure=fig, subplot=[x2,y1,dx,dy])
#	ffits.show_colorscale(cmap='gist_heat', vmin='200', stretch='linear');
	sb2f2.show_colorscale(cmap='hot', vmin=10, vmax=450, stretch='power', smooth=3, kernel='gauss');
	#ffits.show_colorscale(vmin=300, vmax=450, stretch='linear', smooth=3, kernel='gauss');
	sb2f2.add_colorbar();
	a = sb2f2.pixel2world(label_x, label_y);
	if m:
		sb2f2.add_label(a[0], a[1], 'F: 60147094', color='white');
		sb2f2.add_label(b[0], b[1], 'T: '+m.group(1), color='white');
	sb2f2.tick_labels.hide(); # Hide the x and y tick labels
	sb2f2.axis_labels.set_xtext ('East -- l -- West');
	sb2f2.axis_labels.set_ytext ('North -- m -- South');
	sb2f2.frame.set_linewidth(2);
	sb2f2.frame.set_color('green');
#	a = ffits.pixel2world(256, 256);
#	ffits.show_circles (a[0], a[1], a[0]);
	sb2f2.close ();

	sb3f3 = aplpy.FITSFigure(sb1[ind], figure=fig, subplot=[x1,y2,dx,dy])
#	ffits.show_colorscale(cmap='gist_heat', vmin='200', stretch='linear');
	sb3f3.show_colorscale(cmap='hot', vmin=10, vmax=450, stretch='power', smooth=3, kernel='gauss');
	#ffits.show_colorscale(vmin=300, vmax=450, stretch='linear', smooth=3, kernel='gauss');
	sb3f3.add_colorbar();
	a = sb3f3.pixel2world(label_x, label_y);
	if m:
		sb3f3.add_label(a[0], a[1], 'F: 60342407', color='white');
		sb3f3.add_label(b[0], b[1], 'T: '+m.group(1), color='white');
	sb3f3.tick_labels.hide(); # Hide the x and y tick labels
	sb3f3.axis_labels.set_xtext ('East -- l -- West');
	sb3f3.axis_labels.set_ytext ('North -- m -- South');
	sb3f3.frame.set_linewidth(2);
	sb3f3.frame.set_color('green');
#	a = ffits.pixel2world(256, 256);
#	ffits.show_circles (a[0], a[1], a[0]);
	sb3f3.close ();

	sb4f4 = aplpy.FITSFigure(sb1[ind], figure=fig, subplot=[x2, y2,dx,dy])
#	ffits.show_colorscale(cmap='gist_heat', vmin='200', stretch='linear');
	sb4f4.show_colorscale(cmap='hot', vmin=10, vmax=450, stretch='power', smooth=3, kernel='gauss');
	#ffits.show_colorscale(vmin=300, vmax=450, stretch='linear', smooth=3, kernel='gauss');
	sb4f4.add_colorbar();
	a = sb4f4.pixel2world(label_x, label_y);
	if m:
		sb4f4.add_label(a[0], a[1], 'F: 60537719', color='white');
		sb4f4.add_label(b[0], b[1], 'T: '+m.group(1), color='white');
	sb4f4.tick_labels.hide(); # Hide the x and y tick labels
	sb4f4.axis_labels.set_xtext ('East -- l -- West');
	sb4f4.axis_labels.set_ytext ('North -- m -- South');
	sb4f4.frame.set_linewidth(2);
	sb4f4.frame.set_color('green');
#	a = ffits.pixel2world(256, 256);
#	ffits.show_circles (a[0], a[1], a[0]);
	sb4f4.close ();

	fig.savefig (outfile); 
