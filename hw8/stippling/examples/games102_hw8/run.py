import glob
import os
import concurrent.futures

def weight_cvt(file):
    print(file)
    img_name = os.path.basename(file).replace(".bmp","")
    os.system("rm -rf "+img_name)
    os.system("mkdir -p "+img_name)
    os.system("./games102_hw8 "+file+" "+img_name+"/img > "+img_name+"/log.txt")
    os.system("ffmpeg -i "+img_name+"/img%d.svg -r 5 videos/"+img_name+".mp4 -y")

def voronoi(img_name):
    os.system("rm -rf "+img_name)
    os.system("mkdir -p "+img_name)
    os.system("./games102_hw8 empty_path " +img_name+"/img > "+img_name+"/log.txt")
    os.system("ffmpeg -i "+img_name+"/img%d.svg -r 5 videos/"+img_name+".mp4 -y")

def movie_overlay(m1, m2, m3, m4):
    command = "ffmpeg "
    command += "-re -i {} -re -i {} -re -i {} -re -i {} -filter_complex".format(m1, m2, m3, m4)
    command += ''' "nullsrc=size=1024x1024 [base]; [0:v] setpts=PTS-STARTPTS,scale=512x512 [upperleft];   [1:v] setpts=PTS-STARTPTS, scale=512x512 [upperright];
        [2:v] setpts=PTS-STARTPTS, scale=512x512 [lowerleft];
        [3:v] setpts=PTS-STARTPTS, scale=512x512 [lowerright];
        [base][upperleft] overlay=shortest=1[tmp1];
        [tmp1][upperright] overlay=shortest=1:x=512 [tmp2];
        [tmp2][lowerleft] overlay=shortest=1:y=512 [tmp3];
        [tmp3][lowerright] overlay=shortest=1:x=512:y=512" 4x4.mp4'''
    os.system(command)

if __name__ == "__main__":
    os.system("mkdir -p videos")
    voronoi("voronoi")
    with concurrent.futures.ProcessPoolExecutor() as executor:
        for file in executor.map(weight_cvt, list(glob.glob("../data/bmp/*.mp4"))):
            pass
    movie_overlay("videos/lena_gray.mp4", "videos/mao.mp4", "videos/rose.mp4", "videos/sig20.mp4")
