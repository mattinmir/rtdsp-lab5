I = imread('\\icnas1.cc.ic.ac.uk\mm5213\downloads\img.jpg');
imshow(I)
bw = edge(I, 'canny')