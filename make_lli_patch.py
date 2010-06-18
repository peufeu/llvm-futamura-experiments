#! /bin/python

"""
This ugly hack compiles the program and launches it with the local version of lli ; 
when lli complains about an undefined symbol, it modifies lli source code to reference the
symbol as extern and re-compiles lli, so the symbol is included in it and will be found at
runtime.

Imspired by NAKAMURA Takumi.
"""

import subprocess, os.path, re, os

PROG_RUN = ("./lli", "-disable-lazy-compilation", "objs/rpn_module_bitcode.bc")

LLI_MAKE	= ("bash", "compile_lli.sh")
LLI_SOURCE_CODE = "lli.cpp"


def patch( fn, define ):
	fn = "%s(%s);" % (define,fn)
	source = open( LLI_SOURCE_CODE ).read()
	if fn not in source:
		print "Patched lli.cpp : adding", fn
		source = source.replace( "/* INSERT HERE */", ("%s\n/* INSERT HERE */" % fn ))
		open( LLI_SOURCE_CODE, "w" ).write( source )

def run():
	while True:
		p = subprocess.Popen( PROG_RUN, stderr=subprocess.PIPE, stdout=subprocess.PIPE )
		(stdoutdata, stderrdata) = p.communicate()
		print stdoutdata
		print stderrdata
		print p.returncode
		
		fns = re.findall( r"LLVM ERROR: Program used external function '([^']+)' which could not be resolved!", stderrdata )
		for fn in fns:
			patch( fn, "AF" )
		vals = re.findall( r"LLVM ERROR: Could not resolve external global address: ([^\s+])", stderrdata )
		for v in vals:
			patch( v, "AV" )
		
		if not (fns or vals):
			return
		
		p = subprocess.Popen( LLI_MAKE, stderr=subprocess.PIPE, stdout=subprocess.PIPE );
		(stdoutdata, stderrdata) = p.communicate()
		print stdoutdata
		print stderrdata
		if p.returncode:
			print "Compilation error."
			return
	
	
		

run()