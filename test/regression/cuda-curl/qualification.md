# Numerical qualification record

17November2026, isolated CUDA12.6 / arch86 build; Palace baseline6e185f2413d6c4c8c16e0953f0be345342a95e95, libCEEDcbcc9c934bce2efc324fbd2edf95032911109353. No dependency source changes. Manufactured48-tetrahedron order2 cube, analytic B_y=2−i.

|MPI ranks|Device|Mode|Reference action|Exit|Result|
|---|---|---|---|---|---|
|1|cpu|raw|False|0|Pass|
|1|cuda|raw|False|1|Expected original failure|
|1|cuda|raw|True|0|Pass|
|1|cuda|swap|True|0|Pass|
|1|cuda|ref-raw|False|0|Pass|
|4|cpu|raw|False|0|Pass|
|4|cuda|raw|True|0|Pass|
|4|cuda|swap|True|0|Pass|

CPU and opt-in CUDA produce point Re(B_y)=2, Im(B_y)=−1. The unchanged generated CUDA path produces Im(B_y)=0 and fails the assertion in a fresh process. Reversed component order also passes with the workaround. Direct reference action returns a nonzero first result; direct generated libCEED leaf/owned-vector calls reproduce the zero.

Combined observer+curl runtime source9276335af09e6acf9cee3e10c609a3aa30c5b296; executable SHA256752a089f85eab3219a5d845d3968f0d3ef00448321672ead0667576e3ceb3633; base libpalace archivef8ed2e93ac0806166b20b3e254e51c637dc35e83f829c2017d993e68a9d94339; unchanged132-file JIT manifest2499140be6519c05b2a6790e74d7ee95d428b4f03adc6776d19aa7312deadef4. Three translation units(operator, bilinearform and main) recompiled against the frozen remaining archive and dependencies. All CEED operator object allocations occur in the two recompiled FEM files.

Cold transpose duality and forward/additive/transpose entry points passed on one and four CPU/CUDA ranks. The matched one-patch control also completed successfully: same mesh, materials, native ports, passive samples, order2 and relative tolerance1e−10; a previously qualified unshifted complex FGMRES80 preconditioner was used for this control. All complex probes, field energies and surface power flux agree with the frozen CPU reference. Both real and imaginary B are nonzero as expected.

|Native output|Relative vector difference vs CPU|
|---|---|
|probe-E.csv|2.9332e-12|
|probe-B.csv|8.2119e-12|
|port-S.csv|2.6460e-12|
|port-V.csv|3.1332e-12|
|port-I.csv|3.1090e-12|
|domain-E.csv|4.9422e-12|
|surface-F.csv|9.4914e-12|

Native50-ohm S-derived impedance difference: 1.9351e-10ohm.184FGMRES iterations, matching the accepted unshifted control. No patch/feed retuning or physical model adjustment was made. These are small-model equivalence controls, not full-array convergence or fabrication qualification. Exact generated-backend source-line cause is open. This experimental workaround is not an upstream generator repair or an antenna-accuracy qualification.
