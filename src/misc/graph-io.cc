#include <misc/graph-io.h>
#include "kaldi/text-utils.h"

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




fst::StdVectorFst* ReadGraph(std::string filename){
    std::ifstream file( filename.c_str(), std::ios::in|std::ios::binary );
    if (!file) {
        KALDI_ERR << "Could not open decoding-graph FST " << filename;
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
        KALDI_ERR<<"FstHeader::Read: Bad FST header";
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
    StateId s=0;
    float final;
    int64 narcs=0;
    int ilabel;
    int olabel;
    float weight;
    StateId nextstate; 

    fst::StdVectorFst* pfst=new fst::StdVectorFst;

    pfst->SetStart(start);// set start state id to fst

    for(;s<numstates;++s){
        ReadTypeIO(file,final);
        pfst->AddState();
        pfst->SetFinal(s,final);
        ReadTypeIO(file,narcs);
        for(size_t j=0;j<narcs;j++){
            fst::StdArc arc;
            ReadTypeIO(file,arc.ilabel);
            ReadTypeIO(file,arc.olabel);
            ReadTypeIO(file,weight);
            ReadTypeIO(file,arc.nextstate);
            fst::TropicalWeight tmpweight(weight);
            arc.weight=tmpweight;
            pfst->AddArc(s,arc);
        }
    }
    if(s!=numstates){
        KALDI_ERR << "VectorFst::Read: unexpected end of file: ";
        return NULL;
    }
    file.close();
    return pfst;
}

void WriteGraph(fst::StdVectorFst* pfst){

    for(fst::StateIterator<fst::Fst<fst::StdArc> >siter(*pfst);
            !siter.Done();
            siter.Next()){
        fst::StdArc::StateId s=siter.Value();
        for (fst::ArcIterator<fst::Fst<fst::StdArc> > aiter(*pfst, s);
                !aiter.Done();
                aiter.Next()){
            const fst::StdArc& arc=aiter.Value(); 

            std::cout.precision(9);
            if(fabs(arc.weight.Value())<1e-6)
                std::cout<<s<<"\t"<<arc.nextstate<<"\t"<<arc.ilabel<<"\t"<<arc.olabel<<std::endl;
            else
                std::cout<<s<<"\t"<<arc.nextstate<<"\t"<<arc.ilabel<<"\t"<<arc.olabel<<"\t"<<arc.weight<<std::endl;
        }
        if(!isinf(pfst->Final(s).Value())){
            std::cout<<s<<"\t"<<pfst->Final(s)<<std::endl;
        }
    }
}

/*

void PrintCompactLatticeWeight(std::ostream& strm,const kaldi::CompactLatticeWeight& weight){

    strm<<weight.Weight().Value1();
    strm<<",";
    strm<<weight.Weight().Value2();
    strm<<",";
    const std::vector<kaldi::int32>& strs=weight.String();
    if(strs.size()!=0){
        strm<<strs[0];
        for(int i=1;i<strs.size();i++){
            strm<<"_"<<strs[i];
        }
    }
}
void PrintCompactLatticeState(std::ostream& strm,kaldi::CompactLattice& lat,kaldi::CompactLatticeArc::StateId s){

    bool output = false;
    for (fst::ArcIterator< fst::Fst<kaldi::CompactLatticeArc> > aiter(lat, s);
            !aiter.Done();
            aiter.Next()){
        const kaldi::CompactLatticeArc& arc = aiter.Value();

        strm<<s;
        strm<<" ";
        strm<<arc.nextstate;
        strm<<" ";
        strm<<arc.ilabel;

        if(arc.weight != kaldi::CompactLatticeWeight::One()){
            strm<<" ";
            PrintCompactLatticeWeight(strm,arc.weight);
        }
        strm<<std::endl;
        output=true;
    }

    const kaldi::CompactLatticeWeight& final=lat.Final(s);
    if(final != kaldi::CompactLatticeWeight::Zero()||!output){
        strm<<s;
        if(final != kaldi::CompactLatticeWeight::One()){
            strm<<" ";
            PrintCompactLatticeWeight(strm,final);
        }
        strm<<std::endl;
    }

}



bool WriteCompactLattice(std::ostream& strm,std::string key,kaldi::CompactLattice& lat){
    strm<<key<<std::endl;
    StateId start = lat.Start();
    if(start == fst::kNoStateId)
        return false;
    PrintCompactLatticeState(strm,lat,start);
    for(fst::StateIterator<fst::Fst<kaldi::CompactLatticeArc> > siter(lat);
            !siter.Done();
            siter.Next()){
        kaldi::CompactLatticeArc::StateId s=siter.Value();
        if(s != start)
            PrintCompactLatticeState(strm,lat,s);
    }
    strm<<std::endl;
    return true;
}




void ApplyProbabilityScale(float scale, fst::StdMutableFst *fst) {
    //typedef typename StdArc::Weight Weight;
    //typedef typename StdArc::StateId StateId;
    for (fst::StateIterator<fst::StdMutableFst> siter(*fst);
            !siter.Done();
            siter.Next()) {
        fst::StdArc::StateId s = siter.Value();
        for (fst::MutableArcIterator<fst::StdMutableFst> aiter(fst, s);
                !aiter.Done();
                aiter.Next()) {
            fst::StdArc arc = aiter.Value();
            arc.weight = fst::StdArc::Weight(arc.weight.Value() * scale);
            aiter.SetValue(arc);

        }
        if (fst->Final(s) != fst::StdArc::Weight::Zero())
            fst->SetFinal(s, fst::StdArc::Weight(fst->Final(s).Value() * scale));

    }

}

*/



