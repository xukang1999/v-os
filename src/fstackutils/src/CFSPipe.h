#ifndef CFSPIPE_H
#define CFSPIPE_H
namespace fsutils
{
class CFSPipe  
{
public:
    CFSPipe();
    ~CFSPipe();

    CAWResult Open();
    CAWResult Close();

    int GetReadHandle() const;
    int GetWriteHandle() const;
private:
    int m_Handles[2];
};
}//namespace fsutils
#endif // !CMPIPE_H
