IPhw3
=====

A video/image recogonition mfc demo of paper-scissor-stone 

    The program uses HSV color space range to detect skin color ,and open/close (as in morphological operation) with large kernel on resulting binary image to filter out the noise.
    back projection has been used for skin color detection ,but was later abandoned.
    bilateralFilter ,gassianBlur ,and opencv NlMeansDenoising has been tried ,but the results are not ideal ,due to the image retrieved from webcam has terrible quality.
    the program then uses convex hull/defect to find out the gesture.
