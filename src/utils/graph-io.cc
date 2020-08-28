#include <utils/graph-io.h>

std::istream &ReadTypeIO(std::istream &strm, std::string *s) {
    s->clear();
    int32 ns = 0;
    strm.read(reinterpret_cast<char *>(&ns), sizeof(ns));
    for (int i = 0; i < ns; ++i) {
        char c;
        strm.read(&c, 1);
        *s += c;

    }
    return strm;
}
std::istream &ReadTypeIO(std::istream &strm, int32& number ) {
    strm.read(reinterpret_cast<char *>(&number),sizeof(number));
    return strm;
}
std::istream &ReadTypeIO(std::istream &strm, int64& number ) {
    strm.read(reinterpret_cast<char *>(&number),sizeof(number));
    return strm;
}
std::istream &ReadTypeIO(std::istream &strm, uint64& number ) {
    strm.read(reinterpret_cast<char *>(&number),sizeof(number));
    return strm;
}
std::istream &ReadTypeIO(std::istream &strm, float& number ) {
    strm.read(reinterpret_cast<char *>(&number),sizeof(number));
    return strm;
}




athena::StdVectorFst* ReadGraph(std::string filename){
    std::ifstream file( filename.c_str(), std::ios::in|std::ios::binary );
    if (!file) {
        std::cerr << "Could not open decoding-graph FST " << filename;
        return NULL;
    }
    // read fst header infomation
    int32 magic_number;
    std::string fsttype;
    std::string arctype;
    int32 version;
    int32 flags;
    uint64 properties;
    int64 start;
    int64 numstates;
    int64 numarcs;
    ReadTypeIO(file,magic_number);
    if(magic_number != kFstMagicNumber){
        std::cerr<<"FstHeader::Read: Bad FST header";
        return NULL;
    }
    ReadTypeIO(file,&fsttype);
    ReadTypeIO(file,&arctype);
    ReadTypeIO(file,version);
    ReadTypeIO(file,flags);
    ReadTypeIO(file,properties);
    ReadTypeIO(file,start);
    ReadTypeIO(file,numstates);
    ReadTypeIO(file,numarcs);

    // read fst states and arcs
    athena::StdArc::StateId s=0;
    float final;
    int64 narcs=0;
    int ilabel;
    int olabel;
    float weight;
    athena::StdArc::StateId nextstate; 

    athena::StdVectorFst* pfst=new athena::StdVectorFst;

    pfst->SetStart(start);// set start state id to fst

    for(;s<numstates;++s){
        ReadTypeIO(file,final);
        pfst->AddState();
        pfst->SetFinal(s,final);
        ReadTypeIO(file,narcs);
        for(size_t j=0;j<narcs;j++){
            athena::StdArc arc;
            ReadTypeIO(file,arc.ilabel);
            ReadTypeIO(file,arc.olabel);
            ReadTypeIO(file,weight);
            ReadTypeIO(file,arc.nextstate);
            arc.weight.SetValue(weight);
            pfst->AddArc(s,arc);
        }
    }
    if(s!=numstates){
        std::cerr << "VectorFst::Read: unexpected end of file: ";
        return NULL;
    }
    file.close();
    return pfst;
}

void WriteGraph(athena::StdVectorFst* pfst){

    for(athena::StateIterator siter(*pfst);
            !siter.Done();
            siter.Next()){
        athena::StdArc::StateId s=siter.Value();
        for (athena::ArcIterator aiter(*pfst, s);
                !aiter.Done();
                aiter.Next()){
            const athena::StdArc& arc=aiter.Value(); 

            std::cout.precision(9);
            if(fabs(arc.weight.Value())<1e-6)
                std::cout<<s<<"\t"<<arc.nextstate<<"\t"<<arc.ilabel<<"\t"<<arc.olabel<<std::endl;
            else
                std::cout<<s<<"\t"<<arc.nextstate<<"\t"<<arc.ilabel<<"\t"<<arc.olabel<<"\t"<<arc.weight.Value()<<std::endl;
        }
        if(!isinf(pfst->Final(s).Value())){
            std::cout<<s<<"\t"<<pfst->Final(s).Value()<<std::endl;
        }
    }
}


