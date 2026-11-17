# Cold CUDA discrete curl reproduction

Manufactured 48-tetrahedron ND order2 to RT order1 cube. E=(0,0,x) and Im(E)=2Re(E); B=i curl(E) has Re(B_y)=2 and Im(B_y)=-1. No antenna geometry or solver iterations are involved. Pure complex scaling is checked separately.

Build this source against libpalace with the same flags and dependencies as main.cpp. Run fresh processes (no earlier operator application):

```sh
./manufactured cpu raw /path/to/installed/include/palace/
./manufactured cuda raw /path/to/installed/include/palace/
PALACE_CUDA_CURL_REFERENCE=1 ./manufactured cuda raw /path/to/installed/include/palace/
PALACE_CUDA_CURL_REFERENCE=1 ./manufactured cuda swap /path/to/installed/include/palace/
./manufactured cuda ref-raw /path/to/installed/include/palace/
```

The final slash in the JIT source root is required by this Palace build. Run CPU and CUDA controls on one and four MPI ranks. Raw and swap modes assert the manufactured field. The unmodified generated CUDA control is expected to fail on the affected build; keep its output. Other modes investigate direct libCEED and first-use behavior and do not imply passing regression assertions.

The opt-in workaround routes only three-dimensional H(curl) to H(div) discrete interpolator actions through each generated CUDA leaf's reference fallback, including transpose actions. It does not warm up or discard an application, change the main Maxwell operator, enable a global reference backend, or claim to repair the underlying libCEED generator. Default behavior remains unchanged. Generated-backend root-cause analysis remains open.

`actions` additionally checks independent cold transpose/forward actions, transpose duality and both additive entry points. Run it on one and four CPU/CUDA ranks, enabling the explicit workaround for generated CUDA.
