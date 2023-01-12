#pragma once


class Effect;

enum class Technique {
    Point,
    Linear,
    Anisotropic,
    Flat
};

using namespace dae;
class Effect;

struct Vertex_PosCol
{
    Vector3 Pos;
    Vector3 Color;
    Vector2 Uv;
    Vector3 Normal;
    Vector3 Tangent;
    Vector3 viewDirection{};
};

struct Vertex_PosColOut
{
    Vector4 Pos;
    Vector3 Color;
    Vector2 Uv;
    Vector3 Normal;
    Vector3 Tangent;
    Vector3 viewDirection{};
};

enum class PrimitiveTopology
{
    TriangeList,
    TriangleStrip
};

class Mesh
{
public:

    Mesh(ID3D11Device* pDevice, std::vector<Vertex_PosCol> vertices, std::vector<uint32_t> indices);
    ~Mesh();

    void SetMatrix(const dae::Matrix* matrix, const dae::Matrix* worldMatrix, const dae::Vector3* cameraPos);
    void SetWorldMatrix(dae::Matrix matrix) { m_WorldMatrix = matrix; };
    void Render(ID3D11DeviceContext* pDeviceContext);
    //void CycleTechnique();

    Effect* m_pEffect{ nullptr };
    dae::Matrix m_WorldMatrix{};

    //software
    std::vector<Vertex_PosCol> vertices{};
    std::vector<uint32_t> indices{};
private:
    ID3DX11EffectTechnique* m_pTechnique{ nullptr };
    ID3D11InputLayout* m_pInputLayout{ nullptr };
    ID3D11Buffer* m_pVertexBuffer{ nullptr };
    ID3D11Buffer* m_pIndexBuffer{ nullptr };
    int m_NumIndices{ 0 };
    Technique m_Technique{ Technique::Point };

    PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

    std::vector<Vertex_PosColOut> vertices_out{};
  
};

