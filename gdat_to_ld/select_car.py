import os
import sys

hw_config = 'example_hw_config.yaml'

proj_dir = os.getcwd()
main_dir = os.path.dirname(os.path.dirname(proj_dir))
gcannon_path = os.path.join(main_dir,'gophercan-lib','network_autogen')

if (len(sys.argv) < 2):
    print('Need to provide a vehicle config')
    print('Example: python select_car.py go4-23e')
    sys.exit()

car_path = os.path.join(gcannon_path,'configs',sys.argv[1]+'.yaml')
gsense_path = os.path.join(main_dir,'Gopher_Sense')
config_file_path = os.path.join(gsense_path,hw_config)
os.chdir(gcannon_path)
os.system('python3 ' + 'autogen.py' + ' ' + car_path)
os.chdir(gsense_path)
os.system('python3 ' + 'gsense_auto_gen.py' + ' ' + config_file_path)