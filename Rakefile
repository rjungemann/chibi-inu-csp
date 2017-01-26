desc 'Fetch latest utlist.'
task :fetch do
  sh 'cd chibi/inu && rm -f utlist.h'
  sh 'cd chibi/inu && curl -L -O "https://raw.githubusercontent.com/troydhanson/uthash/master/src/utlist.h"'
end

desc 'Try building the stub file for debugging purposes.'
task :build_c do
  sh 'chibi-ffi chibi/inu/csp.stub'
end

desc 'Try building the shared file file for debugging purposes.'
task :build_shared => [:build_c] do
  sh 'cc -fPIC -dynamiclib chibi/inu/csp.c -lchibi-scheme -o chibi/inu/csp.dylib'
end

desc 'Clean the built files.'
task :clean do
  sh 'rm -f chibi/inu/csp.c'
  sh 'rm -f chibi/inu/csp.dylib'
  sh 'rm -f chibi/inu/csp.so'
  sh 'rm -f chibi-inu-csp.tgz'
  sh 'rm -f chibi-inu-csp-test.tgz'
end

desc 'Remove the installed packages.'
task :remove do
  sh 'snow-chibi remove "(chibi inu csp)" >/dev/null 2>&1 || true'
  sh 'snow-chibi remove "(chibi inu csp-test)" >/dev/null 2>&1 || true'
end

desc 'Build the installed packages.'
task :build => [:clean] do
  sh 'snow-chibi --always-no package chibi/inu/csp.sld'
  sh 'snow-chibi --always-no install chibi-inu-csp.tgz'
  sh 'snow-chibi --always-no package chibi/inu/csp-test.sld'
  sh 'snow-chibi --always-no install chibi-inu-csp-test.tgz'
end

desc 'Run the tests.'
task :test do
  sh 'chibi-scheme run-tests.scm'
end

desc 'Build and run.'
task :default => [:build_shared, :test]
