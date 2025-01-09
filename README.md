# Building the project
A requirement for building the project is to have the CPLEX solver installed on your pc. Instructions for this can be found here: https://www.leandro-coelho.com/install-cplex-on-linux-without-administration-privileges.
1. Git clone the repository.
    - Command: ```git clone https://github.com/Ravnholt7507/p9-projekt.git```.
2. Step into the build folder of the repository.
    - Command: ```cd build``` (assuming you are already in the project folder).
3. Create your own version of the makefile (as the one on github might not be up to date).
    - Make sure cmake is installed on your computer, if not it can be installed with ```snap install cmake``` on ubuntu systems.
    - Command: ```cmake ../CMakeLists.txt```.
4. Compile the project. 
    - Command: ```cmake --build .```.

# Running the project
1. To run the project, make sure the output program is executeable.
    - Command: ```chmod +x ./output```.
2. Execute the program
    - Command: ```./output```

# Running unit tests
    - Command: ```./output --run_tests``` 

# Generating graphs
To generate the graph used in the report:
1. Step into the visuals folder (assuming you are in the projects root folder)
    - Command: ```cd visuals``` 
2. Make sure the pip librarys pandas and matplotlib is installed
    - Command ```pip install pandas && pip install matplotlib```
3. Run the python file
    - Command ```python3 plots.py```
