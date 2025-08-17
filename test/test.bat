mkdir windows

cd windows

# test with POCO, some of these will be older to test running ccensus against older code
git clone --depth 1 --branch poco-1.11.8-release https://github.com/pocoproject/poco poco-11
git clone --depth 1 --branch poco-1.12.5-release https://github.com/pocoproject/poco poco-12
git clone --depth 1 --branch poco-1.13.1-release https://github.com/pocoproject/poco poco-13
git clone --depth 1 --branch poco-1.14.2-release https://github.com/pocoproject/poco poco-14

cd poco-11
cmake -B build-windows -S .
cd ..

cd poco-12
cmake -B build-windows -S .
cd ..

cd poco-13
cmake -B build-windows -S .
cd ..

cd poco-14
cmake -B build-windows -S .
cd ..

cd ..

%1\ccensus.exe --json --cmake windows/poco-11/build-windows --output-file poco-11.json
%1\ccensus.exe --json --cmake windows/poco-12/build-windows --output-file poco-12.json
%1\ccensus.exe --json --cmake windows/poco-13/build-windows --output-file poco-13.json
%1\ccensus.exe --json --cmake windows/poco-14/build-windows --output-file poco-14.json

%1\ccensus.exe --json --cmake windows/poco-11/build-windows --output-file poco-actual-11.json
%1\ccensus.exe --json --cmake windows/poco-12/build-windows --output-file poco-actual-12.json
%1\ccensus.exe --json --cmake windows/poco-13/build-windows --output-file poco-actual-13.json
%1\ccensus.exe --json --cmake windows/poco-14/build-windows --output-file poco-actual-14.json

python clip-file.py poco-11.json
python clip-file.py poco-12.json
python clip-file.py poco-13.json
python clip-file.py poco-14.json

fc expected-poco-11.json poco-11.json
fc expected-poco-12.json poco-12.json
fc expected-poco-13.json poco-13.json
fc expected-poco-14.json poco-14.json