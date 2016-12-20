// This file is a part of nla3d project. For information about authors and
// licensing go to project's repository on github:
// https://github.com/dmitryikh/nla3d 

#pragma once
#include "sys.h"

namespace nla3d {

class DofCollection;
// class for Degree of Freedom informations 
// (is it fixed boundary condition, what its number in equation system, ..)
class Dof {
  public:
    enum dofType {
      UX = 0,
      UY,
      UZ,
      ROTX,
      ROTY,
      ROTZ,
      HYDRO_PRESSURE,
      UNDEFINED // should be last
    };

    Dof(dofType t) : eqNumber(0), isConstrained(false), type(t) { }

    uint32 eqNumber = 0; // 0 - not use
    bool isConstrained = false; // is this DOF defined by B.C.
    dofType type;

    static const char* const dofTypeLabels[];
    static const uint16 numberOfDofTypes;
    static dofType label2dofType (const std::string& label);
    static const char* dofType2label (const dofType t);
};

inline const char* Dof::dofType2label(const Dof::dofType t) {
  assert(t < Dof::UNDEFINED);
  return dofTypeLabels[t];
}


class DofCollection {
  public:
    uint32 getNumberOfUsedDofs();
    uint32 getNumberOfEntities();
    Dof* getDof(uint32 n, Dof::dofType dof);

    // NOTE: Dof add procudere rely on the fact that n (entity number) is growing from 1 
    // to numberOfEntities. If storage will add 
    void initDofTable(uint32 _numberOfEntities);
    void addDof(uint32 n, Dof::dofType dof);
    void clearDofTable();

    bool isDofUsed(uint32 n, Dof::dofType dof);

  private:
    uint32 numberOfUsedDofs = 0;
    uint32 numberOfEntities = 0;

    // row of Dof objects 
    std::vector<Dof> dofs;
    // array of indexes to find where dofs for particular entity is located in dofs
    // Dof for entity n will be located from dofPos[n-1] included to dofPos[n] excluded 
    std::vector<uint32> dofPos;
    static uint32 empty;
};


inline uint32 DofCollection::getNumberOfUsedDofs() {
  return numberOfUsedDofs;
}


inline uint32 DofCollection::getNumberOfEntities() {
  return numberOfEntities;
}



} // namespace nla3d 
