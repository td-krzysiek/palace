// Minimal manufactured-field diagnostic; no antenna solve or input modification.
#include <mfem.hpp>
#include <complex>
#include <iostream>
#include <ceed/backend.h>
#include "fem/fespace.hpp"
#include "fem/gridfunction.hpp"
#include "fem/integrator.hpp"
#include "fem/mesh.hpp"
#include "fem/libceed/ceed.hpp"
#include "fem/libceed/operator.hpp"
#include "linalg/hypre.hpp"
#include "linalg/rap.hpp"
#include "linalg/vector.hpp"
#include "utils/communication.hpp"
#include "utils/device.hpp"
using namespace palace;
int main(int argc,char **argv)
{
  Mpi::Init(argc,argv);
  std::cerr<<"stage MPI\n";
  std::string backend=argc>1?argv[1]:"cpu";
  std::string mode=argc>2?argv[2]:"raw";
  utils::ConfigureMemory(backend,0);
  mfem::Device device(backend.c_str(),0);
  std::cerr<<"stage device\n";
  hypre::Initialize();
  ceed::Initialize(backend=="cpu"?"/cpu/self":(mode=="ref-raw"?"/gpu/cuda/ref":"/gpu/cuda/gen"),(argc>3?argv[3]:"./"));
  std::cerr<<"stage CEED\n";
  fem::DefaultIntegrationOrder::p_trial=2;
  {
    ComplexVector scale(5);scale.UseDevice(true);scale.Real()=2.;scale.Imag()=3.;scale*=std::complex<double>(0.,1.);
    std::cerr<<"stage scaled\n";
    const double sr=scale.Real().HostRead()[0],si=scale.Imag().HostRead()[0];
    std::cerr<<"stage scale read "<<sr<<","<<si<<"\n";
    auto serial=mfem::Mesh::MakeCartesian3D(2,2,2,mfem::Element::TETRAHEDRON);
    serial.EnsureNodes();
    Mesh mesh(Mpi::World(),serial);
    std::cerr<<"stage mesh\n";
    mfem::ND_FECollection nd_fec(2,3);mfem::RT_FECollection rt_fec(1,3);
    FiniteElementSpace nd(mesh,&nd_fec),rt(mesh,&rt_fec);
    mfem::ParGridFunction gf(&nd.Get());
    mfem::VectorFunctionCoefficient e_coef(3,[](const mfem::Vector &x,mfem::Vector &v){v=0.;v(2)=x(0);});
    gf.ProjectCoefficient(e_coef);
    const auto &curl=rt.GetDiscreteInterpolator(nd);
    std::cerr<<"stage curl built\n";
    ComplexVector e(curl.Width()),b(curl.Height());e.UseDevice(true);b.UseDevice(true);
    gf.GetTrueDofs(e.Real());e.Imag()=e.Real();e.Imag()*=2.;b=0.;
    if(mode=="restriction-first")rt.GetRestrictionMatrix();
    if(mode=="local-first")
    {
      const auto &local=dynamic_cast<const ParOperator &>(curl).LocalOperator();
      Vector out(rt.GetVSize());out.UseDevice(true);local.Mult(gf,out);
      std::cerr<<"local first norm "<<out.Norml2()<<"\n";
    }
    if(mode=="ceed-owned"||mode=="ceed-leaf"||mode=="ceed-apply"||mode=="ceed-fallback")
    {
      const auto &local=dynamic_cast<const ceed::Operator &>(dynamic_cast<const ParOperator &>(curl).LocalOperator());
      Ceed context=ceed::internal::GetCeedObjects()[0];CeedVector in,out;
      CeedVectorCreate(context,nd.GetVSize(),&in);CeedVectorCreate(context,rt.GetVSize(),&out);
      CeedVectorSetArray(in,CEED_MEM_HOST,CEED_COPY_VALUES,const_cast<double *>(gf.HostRead()));CeedVectorSetValue(out,0.);
      if(mode=="ceed-fallback") {CeedOperator *subs,ref;CeedOperatorCompositeGetSubList(local[0],&subs);CeedOperatorGetFallback(subs[0],&ref);CeedOperatorApplyAdd(ref,in,out,CEED_REQUEST_IMMEDIATE);}
      else if(mode=="ceed-leaf") {CeedOperator *subs;CeedOperatorCompositeGetSubList(local[0],&subs);CeedOperatorApplyAdd(subs[0],in,out,CEED_REQUEST_IMMEDIATE);}
      else if(mode=="ceed-apply")CeedOperatorApply(local[0],in,out,CEED_REQUEST_IMMEDIATE);
      else CeedOperatorApplyAdd(local[0],in,out,CEED_REQUEST_IMMEDIATE);
      const CeedScalar *a;CeedVectorGetArrayRead(out,CEED_MEM_HOST,&a);double norm=0.;for(int i=0;i<rt.GetVSize();i++)norm+=a[i]*a[i];
      std::cerr<<"CEED owned first squared norm "<<norm<<"\n";CeedVectorRestoreArrayRead(out,&a);CeedVectorDestroy(&in);CeedVectorDestroy(&out);
    }
    if(mode=="warmup") {Vector tmp(curl.Height());tmp.UseDevice(true);curl.Mult(e.Real(),tmp);}
    if(mode=="swap") {curl.Mult(e.Imag(),b.Imag());curl.Mult(e.Real(),b.Real());}
    else {curl.Mult(e.Real(),b.Real());if(mode=="read-first")b.Real().HostRead();curl.Mult(e.Imag(),b.Imag());}
    b*=std::complex<double>(0.,1.);
    std::cerr<<"stage curl applied\n";
    const double nr=b.Real().Norml2(),ni=b.Imag().Norml2();
    GridFunction field(rt,true);field.Real().SetFromTrueDofs(b.Real());field.Imag().SetFromTrueDofs(b.Imag());
    mfem::Vector vr(3),vi(3);const auto &ip=mfem::Geometries.GetCenter(mfem::Geometry::TETRAHEDRON);
    field.Real().GetVectorValue(0,ip,vr);field.Imag().GetVectorValue(0,ip,vi);
    const bool correct = std::abs(sr+3.)<1.e-12 && std::abs(si-2.)<1.e-12 && std::abs(vr(1)-2.)<1.e-10 && std::abs(vi(1)+1.)<1.e-10;
    if (mode=="raw" || mode=="swap" || mode=="ref-raw") MFEM_VERIFY(correct,"Cold discrete curl must preserve both complex components");
    if(Mpi::Root(Mpi::World()))std::cout<<"REPRO_JSON {\"device\":\""<<backend<<"\",\"mode\":\""<<mode<<"\",\"scaleRe\":"<<sr<<",\"scaleIm\":"<<si<<",\"normRe\":"<<nr<<",\"normIm\":"<<ni<<",\"pointReY\":"<<vr(1)<<",\"pointImY\":"<<vi(1)<<"}\n";
  }
  ceed::Finalize();Mpi::Finalize();return 0;
}
