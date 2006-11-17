#include <algorithm>
#include "str.hh"

template <typename KT>
bool ClusterMap<KT>::init_order(const int order) {
  int old_order=m_map.size()-1;
  m_map.resize(order+1);
  m_num_cl.resize(order+1,1);
  //fprintf(stderr,"Resized m_map from %d to %d\n", old_order,m_map.size());

  if (old_order<order) {
    if (order==1) {
      m_map[order].resize(num_words(),0);
      m_num_cl[1]=num_words();
    } else {
      m_map[order].resize(num_clusters(order-1),0);
    }
  }
  if (order!=1) {
    if (old_order>=order) return(false);
    return(true);
  }

  for (KT i=0;i<num_words();i++) {
    m_map[1][i]=i;
  }
  return(true);
}

template <typename KT>
void ClusterMap<KT>::write(FILE *out, int order) {
  if (order==0 || order ==1) {
    if (order==0) fprintf(out,"\\clustermap %d\n",m_map.size()-1);
    fprintf(out,"\\1-ords %d\n", num_words());
    for (int j=0;j<num_words();j++) 
      fprintf(out,"%s %d\n",word(j).c_str(),get_cluster(1,j));
    if (order==1) return;
  }
  
  int from = order;
  int to = order+1;
  if (!order) {
    from = 2;
    to = m_map.size();
  }

  for (int o=from;o<to;o++) {
    fprintf(out,"\\%d-ords %d\n", o, num_clusters(o));
    const int nc=num_clusters(o-1);
    for (int j=0;j<nc;j++) {
      if (o==1) fprintf(out,"%s ",word(j).c_str());
      else fprintf(out,"%d ",j);
      fprintf(out,"%d\n",get_cluster2(o,j));
    }
  }
  if (!order)
    fprintf(out,"\\endcl\n\n");
}

template <typename KT>
int ClusterMap<KT>::read(FILE *in, const int ord, int read_lines) {
  // Assuming that "\\clustermap %d" has already been read
  std::string s;
  std::vector<std::string> split_s;
  int o=0,wi;
  bool ok=true;
  int num_cl, old_cl=0;
  m_map.resize(ord+1);
  m_num_cl.resize(ord+1);

  read_lines++;
  if (!str::read_line(&s,in,true)) read_error(read_lines,s);
  while (s!="\\endcl") {
    // Read \n-ords %d (%d)
    str::split(&s, " ", true, &split_s, 3);
    int o2;
    sscanf(s.c_str(),"\\%d-ords",&o2);
    if (++o!=o2) read_error(read_lines,s);

    if (split_s.size()!=2) read_error(read_lines,s);

    num_cl=str::str2long(&(split_s[1]),&ok);
    m_num_cl[o]=num_cl;
    if (o==1) old_cl=num_cl;
    m_map[o].resize(old_cl);
    old_cl=num_cl;

    if (!str::read_line(&s,in,true)) read_error(read_lines,s);
    while (s[0]!='\\') {
      read_lines++;      
      str::split(&s," ",true, &split_s, 4);
      if (split_s.size()!=2) read_error(read_lines,s);
      if (o==1) wi=add_word(split_s[0].c_str());
      else {
	wi=str::str2long(&(split_s[0]),&ok);
	assert(wi<m_map[o].size());
      }
  
      int tmp=str::str2long(&(split_s[1]),&ok);
      if (tmp>=0) {
	if (tmp>=num_cl) read_error(read_lines, s);
	set_cluster(o, wi, 0, tmp);
      }
      if (!str::read_line(&s,in,true)) read_error(read_lines,s);
    }
  }
  return(read_lines);
}

template <typename KT>
void ClusterMap<KT>::wv2cv(std::vector<KT> &v) {
  const int order=v.size();
  const int maxclo=m_map.size()-1;
  //fprintf(stderr,"MAPPING ");print_indices(v);
  for (int i=1;i<=order;i++) {
    v[order-i]=get_cluster(std::min(i,maxclo),v[order-i]);
  }
  //fprintf(stderr,"->");print_indices(v);fprintf(stderr,"\n");
}

template <typename KT>
void ClusterMap<KT>::wg2cg(NGram::Gram &g) {
  const int order=g.size();
  const int maxclo=m_map.size()-1;
  for (int i=1;i<=order;i++) {
    g[order-i]=get_cluster(std::min(i, maxclo),g[order-i]);
  }
}

template <typename KT>
void ClusterMap<KT>::read_error(const int line, const std::string &text) {
  fprintf(stderr,"Error reading clustermap, line %d:\n",line);
  fprintf(stderr,"%s\n", text.c_str());
  fprintf(stderr,"Exit.\n");
  exit(-1);
}

#if 0
template <typename KT>
void ClusterMap<KT>::init_backwards_map() {
  backmap.resize(m_map.size());
  for (int o=2;o<m_map.size();o++) {
    backmap[o].resize(o-1);
    
    // Build the mapping to next level of clusters
    for (int o2=1;o2<o;o2++)
      backmap[o][o2].resize(num_clusters(o));

    for (int i=0;i<num_clusters(o);i++) 
      backmap[o][o-1][get_cluster2(i)]=i;
    
    for (int o2=o-2;o2>=1;o2++) {
      backmap[o][o2].resize(num_clusters(o));
      for (int i=0;i<num_clusters(o);i++) {
	for (int j=0;j<bacmap[o][o-1]
      }
    }
	     
}
#endif

/************************************************************************/
/* The inlined functions                                                */
/************************************************************************/
template <typename KT>
void ClusterMap<KT>::set_cluster(const int order, const KT lowcl,
				    const KT oricl, const KT newcl) {
  // Should assert things here
  m_map[order][lowcl]=newcl;

  if (newcl >= m_num_cl[order]) m_num_cl[order]=newcl+1;
  if (oricl >= m_num_cl[order]-1) {
    m_num_cl[order]=*(max_element(m_map[order].begin(),m_map[order].end()))+1;
    //fprintf(stderr,"SET STUFF %d: %d(%d)\n", order, m_num_cl[order], num_clusters(order));
  }
}

template <typename KT>
KT ClusterMap<KT>::get_cluster(const int order, KT w) {
  for (int i=1;i<=order;i++) {
    w=m_map[i][w];
  }
  return(w);
}

template <typename KT>
KT ClusterMap<KT>::get_cluster2(const int order, const KT cl) {
  return(m_map[order][cl]);
} 

template <typename KT>
void ClusterFMap<KT>::set_fcluster(const int order, const KT lowcl,
				      const KT oricl, const KT newcl) {
  // Should assert things here
  m_fmap[order][lowcl].key=newcl;

  if (newcl >= m_num_fcl[order]) m_num_fcl[order]=newcl+1;
  if (oricl >= m_num_fcl[order]-1)
    m_num_fcl[order]=max_element(m_fmap[order].begin(),m_fmap[order].end())->key+1;
  //fprintf(stderr,"CUR MAXFC %d: %d (%d)\n", order, m_num_fcl[order], newcl);
}

template <typename KT>
KT ClusterFMap<KT>::get_fcluster(const int order, KT w) {
  for (int i=1;i<=order;i++) {
    w=m_fmap[i][w].key;
  }
  return(w);
}

template <typename KT>
float ClusterFMap<KT>::get_fprob(const int order, KT w) {
  return(m_fmap[order][get_fcluster(order-1,w)].lprob);
}

template <typename KT>
float ClusterFMap<KT>::get_fprob2(const int order, KT w) {
  return(m_fmap[order][w].lprob);
}

template <typename KT>
float ClusterFMap<KT>::get_full_emprob(const int order, KT w) {
  float lprob=0;
  KT w2=w;
  for (int i=2;i<=order;i++) {
    w2=get_fcluster2(i-1,w2);
    lprob+=m_fmap[i][w2].lprob;
  }
  return(lprob);
}

template <typename KT>
void ClusterFMap<KT>::set_fprob(const int order, KT w, const float pr) {
  m_fmap[order][w].lprob=pr;
}

template <typename KT>
KT ClusterFMap<KT>::get_fcluster2(const int order, const KT cl) {
  return(m_fmap[order][cl].key);
} 

template <typename KT>
bool ClusterFMap<KT>::init_order(const int order) {
  bool resize=ClusterMap<KT>::init_order(order);

  m_fmap.resize(order+1);
  m_num_fcl.resize(order+1,1);

  if (resize) {
    if (order==1) {
      m_fmap[order].resize(Vocabulary::num_words());
      m_num_fcl[1]=Vocabulary::num_words();
    } else {
      m_fmap[order].resize(num_fclusters(order-1));
    }
  }
  if (order!=1 || !resize) return(resize);;

  for (KT i=0;i<Vocabulary::num_words();i++) {
    key_prob &k=m_fmap[1][i];
    k.key=i;
    k.lprob=0.0;
  }
  return(resize);
}


template <typename KT>
void ClusterFMap<KT>::write(FILE *out) {
  fprintf(out,"\\fclustermap %d\n",ClusterMap<KT>::m_map.size()-1);
  
  fprintf(out,"\\1-ords %d %d\n", Vocabulary::num_words(), Vocabulary::num_words());
  for (int j=0;j<Vocabulary::num_words();j++) {
    fprintf(out,"%s %d %d %.3f\n",Vocabulary::word(j).c_str(),ClusterMap<KT>::get_cluster(1,j),get_fcluster(1,j), get_fprob(1,j));
  }

  for (int o=2;o<ClusterMap<KT>::m_map.size();o++) {
    fprintf(out,"\\%d-ords %d %d\n", o, ClusterMap<KT>::num_clusters(o), num_fclusters(o));
    const int nc=ClusterMap<KT>::num_clusters(o-1);
    const int nfc=num_fclusters(o-1);
    const int looptill=std::max(nc,nfc);
    for (int j=0;j<looptill;j++) {
      if (o==1) fprintf(out,"%s ",Vocabulary::word(j).c_str());
      else fprintf(out,"%d ",j);
      if (j<nc) fprintf(out,"%d ",ClusterMap<KT>::get_cluster2(o,j));
      else fprintf(out,"-1 ");
      if (j<nfc) fprintf(out,"%d %.3f\n",get_fcluster2(o,j),get_fprob2(o,j));
      else fprintf(out,"-1 0\n");
    }
  }
  fprintf(out,"\\endcl\n\n");
}

template <typename KT>
int ClusterFMap<KT>::read(FILE *in, const int ord, int read_lines) {
  // Assuming that "\\fclustermap %d" has already been read
  std::string s;
  std::vector<std::string> split_s;
  int o=0,wi;
  bool ok=true;
  int num_cl, old_cl=0;
  int num_fcl, old_fcl=0;
  m_fmap.resize(ord+1);
  m_num_fcl.resize(ord+1);
  ClusterMap<KT>::m_map.resize(ord+1);
  ClusterMap<KT>::m_num_cl.resize(ord+1);

  read_lines++;
  if (!str::read_line(&s,in,true)) ClusterMap<KT>::read_error(read_lines,s);
  while (s!="\\endcl") {
    // Read \n-ords %d (%d)
    str::split(&s, " ", true, &split_s, 3);
    int o2;
    sscanf(s.c_str(),"\\%d-ords",&o2);
    if (++o!=o2) ClusterMap<KT>::read_error(read_lines,s);

    if (split_s.size()!=3) ClusterMap<KT>::read_error(read_lines,s);
    num_fcl=str::str2long(&(split_s[2]),&ok);
    m_num_fcl[o]=num_fcl;
    if (o==1) old_fcl=num_fcl;
    m_fmap[o].resize(old_fcl); 
    old_fcl=num_fcl;

    num_cl=str::str2long(&(split_s[1]),&ok);
    ClusterMap<KT>::m_num_cl[o]=num_cl;
    if (o==1) old_cl=num_cl;
    ClusterMap<KT>::m_map[o].resize(old_cl);
    old_cl=num_cl;

    read_lines++;
    if (!str::read_line(&s,in,true)) ClusterMap<KT>::read_error(read_lines,s);
    while (s[0]!='\\') {
      read_lines++;      
      str::split(&s," ",true, &split_s, 4);
      if (split_s.size()!=4) ClusterMap<KT>::read_error(read_lines,s);
      if (o==1) wi=Vocabulary::add_word(split_s[0].c_str());
      else {
	wi=str::str2long(&(split_s[0]),&ok);
	assert(wi< std::max(ClusterMap<KT>::m_map[o-1].size(), m_fmap[o-1].size()));
      }
  
      int tmp=str::str2long(&(split_s[1]),&ok);
      if (tmp>=0) {
	assert(tmp<num_cl);
	ClusterMap<KT>::set_cluster(o, wi, 0, tmp);
      } else if (wi<ClusterMap<KT>::m_map[o].size()) {
	fprintf(stderr,"a ");ClusterMap<KT>::read_error(read_lines,s);
      }

      tmp=str::str2long(&(split_s[2]),&ok);
      if (!ok) ClusterMap<KT>::read_error(read_lines,s);
      if (tmp>=0) {
	assert(tmp<num_fcl);
	set_fcluster(o, wi, 0, tmp);
	set_fprob(o, wi, str::str2float(&(split_s[3]),&ok));
	if (!ok) ClusterMap<KT>::read_error(read_lines,s);
      } else if (wi<m_fmap[o].size()) {
	fprintf(stderr,"b ");ClusterMap<KT>::read_error(read_lines,s);
      }

      if (!str::read_line(&s,in,true)) ClusterMap<KT>::read_error(read_lines,s);
    }
  }
  return(read_lines);
}

template <typename KT>
void ClusterMap<KT>::read_more(FILE *file) {
  // Mostly copied from TreeGramArpaReader.cc
  std::string line;
  std::vector<std::string> vec;

  // Just for efficiency
  line.reserve(128); 
  vec.reserve(16);

  bool ok = true;

  int lineno =0;

  // Find header
  while (1) {
    ok = str::read_line(&line, file, true);
    lineno++;

    if (!ok) {
      fprintf(stderr, "ClusterMap::read_more(): "
              "error on line %d while waiting \\data\\", lineno);
      exit(1);
    }

    if (line.substr(0,11) == "\\clustermap") {
      int ord;
      if (sscanf(line.c_str(),"\\clustermap %d",&ord)!=1) assert(false);
      lineno=read(file,ord,lineno);
      for (int i=0;i<num_words();i++) {
        add_word(word(i));
      }
      return;
    } 

    if (line.substr(0,12) == "\\fclustermap") {
      assert(false);
    } 

    if (line == "\\data\\")
      break;
  }
  fprintf(stderr,"read_more: error, no clusters found.\n");
  exit(-1);
}
