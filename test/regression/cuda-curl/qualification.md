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

Matched one-patch and additive/transpose qualification are pending at this checkpoint. Exact generated-backend source-line cause is open. This experimental workaround is not an upstream generator repair or an antenna-accuracy qualification.
