#include "StdInc.h"

#include "ColModel.h"

void CColModel::InjectHooks()
{
    RH_ScopedClass(CColModel);
    RH_ScopedCategory("Collision");

    RH_ScopedInstall(operator new, 0x40FC30);
    RH_ScopedInstall(operator delete, 0x40FC40);
    RH_ScopedInstall(MakeMultipleAlloc, 0x40F740);
    RH_ScopedOverloadedInstall(AllocateData, "void", 0x40F810, void(CColModel::*)());
    RH_ScopedOverloadedInstall(AllocateData, "params", 0x40F870, void(CColModel::*)(int32, int32, int32, int32, int32, bool));
    RH_ScopedInstall(RemoveCollisionVolumes, 0x40F9E0);
    RH_ScopedInstall(CalculateTrianglePlanes, 0x40FA30);
    RH_ScopedInstall(RemoveTrianglePlanes, 0x40FA40);
}

CColModel::CColModel() : m_boundBox()
{
    m_nColSlot = 0;
    m_pColData = nullptr;
    m_bNotEmpty             = false;
    m_bIsSingleColDataAlloc = false;
    m_bIsActive             = true;
}

CColModel::~CColModel()
{
    if (!m_bIsActive)
        return;

    RemoveCollisionVolumes();
}

CColModel& CColModel::operator=(const CColModel& colModel)
{
    //BUG(Prone) No self assignment check
    m_boundSphere.m_vecCenter = colModel.m_boundSphere.m_vecCenter;
    m_boundSphere.m_fRadius = colModel.m_boundSphere.m_fRadius;
    m_boundBox = colModel.m_boundBox;
    if (m_pColData)
        m_pColData->Copy(*colModel.m_pColData);

    return *this;
}

void CColModel::MakeMultipleAlloc()
{
    if (!m_bIsSingleColDataAlloc)
        return;

    auto* colData = new CCollisionData();
    colData->Copy(*m_pColData);
    delete m_pColData;

    m_bIsSingleColDataAlloc = false;
    m_pColData = colData;
}

void CColModel::AllocateData()
{
    m_bIsSingleColDataAlloc = false;
    m_pColData = new CCollisionData();
}

// Memory layout of m_pColData is: | CCollisionData | CColSphere[] | CColLine[]/CColDisk[] | CColBox[] | Vertices[] | CColTriangle[] |
void CColModel::AllocateData(int32 numSpheres, int32 numBoxes, int32 numLines, int32 numVertices, int32 numTriangles, bool bUsesDisks)
{
    const auto baseSize = sizeof(CCollisionData);
    const auto spheresSize = numSpheres * sizeof(CColSphere);
    const auto linesOrDisksSize = bUsesDisks ? (numLines * sizeof(CColDisk)) : (numLines * sizeof(CColLine));
    const auto vertsSize = numVertices * sizeof(CompressedVector);
    const auto boxesSize = numBoxes * sizeof(CColBox);
    const auto trianglesSize = numTriangles * sizeof(CColTriangle);

    const auto spheresOffset = baseSize;
    const auto linesOrDisksOffset = spheresOffset + spheresSize;
    const auto boxesOffset = linesOrDisksOffset + linesOrDisksSize;
    const auto vertsOffset = boxesOffset + boxesSize;

    // ORIGNAL ALIGNMENT LOGIC, with possible bug
    // BUG?: Seems like it could clip the last 3 bytes of vertices array
    //  const auto trianglesOffset = (vertsOffset + vertsSize) & 0xFFFFFFFC; // Align the offset to 4 bytes boundary

    auto* pTrisStart = reinterpret_cast<void*>(vertsOffset + vertsSize);
    auto space = trianglesSize + 4;
    auto* pAlignedAddress = std::align(4, trianglesSize, pTrisStart, space);// 4 bytes aligned address
    const auto trianglesOffset = reinterpret_cast<uint32>(pAlignedAddress);
    assert(trianglesOffset && trianglesOffset >= (vertsOffset + vertsSize)); // Just to make sure that the alignment works properly

    AllocateData(trianglesOffset + trianglesSize);
    m_pColData->m_nNumSpheres = numSpheres;
    m_pColData->m_nNumLines = numLines;
    m_pColData->m_nNumBoxes = numBoxes;
    m_pColData->m_nNumTriangles = numTriangles;

    m_pColData->m_pSpheres = m_pColData->GetPointerToColArray<CColSphere>(spheresOffset);
    if (bUsesDisks)
        m_pColData->m_pDisks = numLines ? m_pColData->GetPointerToColArray<CColDisk>(linesOrDisksOffset) : nullptr;
    else
        m_pColData->m_pLines = numLines ? m_pColData->GetPointerToColArray<CColLine>(linesOrDisksOffset) : nullptr;

    m_pColData->m_pBoxes = numBoxes ? m_pColData->GetPointerToColArray<CColBox>(boxesOffset) : nullptr;
    m_pColData->m_pVertices = numVertices ? m_pColData->GetPointerToColArray<CompressedVector>(vertsOffset) : nullptr;
    m_pColData->m_pTriangles = numTriangles ? m_pColData->GetPointerToColArray<CColTriangle>(trianglesOffset) : nullptr;
    m_pColData->m_pTrianglePlanes = nullptr;

    m_pColData->bUsesDisks = bUsesDisks;
}

void CColModel::AllocateData(int32 size)
{
    m_bIsSingleColDataAlloc = true;
    m_pColData = static_cast<CCollisionData*>(CMemoryMgr::Malloc(size));
}

void CColModel::RemoveCollisionVolumes()
{
    if (!m_pColData)
        return;

    if (m_bIsSingleColDataAlloc) {
        CCollision::RemoveTrianglePlanes(m_pColData);
        CMemoryMgr::Free(m_pColData);
    }
    else
    {
        m_pColData->RemoveCollisionVolumes();
        delete m_pColData;
    }

    m_pColData = nullptr;
}

void CColModel::CalculateTrianglePlanes()
{
    if (m_pColData)
        m_pColData->CalculateTrianglePlanes();
}

void CColModel::RemoveTrianglePlanes()
{
    if (m_pColData)
        m_pColData->RemoveTrianglePlanes();
}

void* CColModel::operator new(unsigned size) {
    return GetColModelPool()->New();
}

void CColModel::operator delete(void* data) {
    GetColModelPool()->Delete(static_cast<CColModel*>(data));
}
