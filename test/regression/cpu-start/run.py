"""Run the real CUDA/HIP-enabled executable without accelerator devices.

Usage: python3 run.py /path/to/palace /path/to/mpirun /fresh/output
Run inside a device-free container; this is not a --dry-run check.
"""
from pathlib import Path
import json,os,subprocess,sys
binary,mpi,out=Path(sys.argv[1]),Path(sys.argv[2]),Path(sys.argv[3]);out.mkdir()
assert not list(Path('/dev').glob('nvidia[0-9]*'))
fixture=Path(__file__).resolve().parent
config=dict(Problem=dict(Type='Electrostatic',Verbose=2,Output=str(out/'results')),
 Model=dict(Mesh=str(fixture/'cube.mesh'),L0=1.0),
 Domains=dict(Materials=[dict(Attributes=[1],Permittivity=1.0)]),
 Boundaries=dict(Ground=dict(Attributes=[1]),Terminal=[dict(Index=1,Attributes=[2])]),
 Solver=dict(Device='CPU',Backend='/cpu/self',Order=2,Electrostatic=dict(Save=0),
 Linear=dict(Type='BoomerAMG',KSPType='CG',Tol=1e-10,MaxIts=100)))
(out/'config.json').write_text(json.dumps(config,indent=2)+'\n')
env=dict(os.environ,CUDA_VISIBLE_DEVICES='',HIP_VISIBLE_DEVICES='',OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',OMPI_ALLOW_RUN_AS_ROOT='1',OMPI_ALLOW_RUN_AS_ROOT_CONFIRM='1')
with(out/'solver.log').open('w')as log:
 result=subprocess.run([str(mpi),'--bind-to','none','-n','2',str(binary),str(out/'config.json')],env=env,stdout=log,stderr=subprocess.STDOUT,timeout=120)
text=(out/'solver.log').read_text()
assert result.returncode==0,text[-2500:]
assert 'Device configuration: cpu'in text and 'CG solver converged'in text
assert 'CUDA error:'not in text and 'HIP error:'not in text
assert list((out/'results').glob('*.csv'))
print(json.dumps(dict(status='PASS',mpiRanks=2,executionDevice='CPU',gpuDeviceNodes=[],exitCode=result.returncode)))
