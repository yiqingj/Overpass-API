/** Copyright 2008, 2009, 2010, 2011, 2012 Roland Olbricht
*
* This file is part of Overpass_API.
*
* Overpass_API is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* Overpass_API is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with Overpass_API.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DE__OSM3S___OVERPASS_API__DATA__COLLECT_MEMBERS_H
#define DE__OSM3S___OVERPASS_API__DATA__COLLECT_MEMBERS_H

#include "../core/datatypes.h"
#include "../statements/statement.h"
#include "abstract_processing.h"
#include "filenames.h"

#include <map>
#include <set>
#include <vector>

using namespace std;

class Resource_Manager;
class Statement;


map< Uint31_Index, vector< Relation_Skeleton > > relation_relation_members
    (const Statement& stmt, Resource_Manager& rman,
     const map< Uint31_Index, vector< Relation_Skeleton > >& parents,
     const set< pair< Uint31_Index, Uint31_Index > >* children_ranges = 0,
     const vector< Relation::Id_Type >* children_ids = 0, bool invert_ids = false, const uint32* role_id = 0);

map< Uint31_Index, vector< Way_Skeleton > > relation_way_members
    (const Statement* stmt, Resource_Manager& rman,
     const map< Uint31_Index, vector< Relation_Skeleton > >& relations,
     const set< pair< Uint31_Index, Uint31_Index > >* way_ranges = 0,
     const vector< Way::Id_Type >* way_ids = 0, bool invert_ids = false, const uint32* role_id = 0);

map< Uint32_Index, vector< Node_Skeleton > > relation_node_members
    (const Statement* stmt, Resource_Manager& rman,
     const map< Uint31_Index, vector< Relation_Skeleton > >& relations,
     const set< pair< Uint32_Index, Uint32_Index > >* node_ranges = 0,
     const vector< Node::Id_Type >* node_ids = 0, bool invert_ids = false, const uint32* role_id = 0);
 
map< Uint32_Index, vector< Node_Skeleton > > way_members
    (const Statement* stmt, Resource_Manager& rman,
     const map< Uint31_Index, vector< Way_Skeleton > >& ways,
     const set< pair< Uint32_Index, Uint32_Index > >* node_ranges = 0,
     const vector< Node::Id_Type >* node_ids = 0, bool invert_ids = false);

     
vector< Node::Id_Type > relation_node_member_ids
    (Resource_Manager& rman, const map< Uint31_Index, vector< Relation_Skeleton > >& rels,
     const uint32* role_id = 0);
vector< Way::Id_Type > relation_way_member_ids
    (Resource_Manager& rman, const map< Uint31_Index, vector< Relation_Skeleton > >& rels,
     const uint32* role_id = 0);
vector< Relation::Id_Type > relation_relation_member_ids
    (Resource_Manager& rman, const map< Uint31_Index, vector< Relation_Skeleton > >& rels,
     const uint32* role_id = 0);

vector< Node::Id_Type > way_nd_ids(const map< Uint31_Index, vector< Way_Skeleton > >& ways);

const map< uint32, string >& relation_member_roles(Transaction& transaction);
uint32 determine_role_id(Transaction& transaction, const string& role);


void add_way_to_area_blocks(const vector< Quad_Coord >& coords,
                            uint32 id, map< Uint31_Index, vector< Area_Block > >& areas);


vector< Quad_Coord > make_geometry(const Way_Skeleton& way, const vector< Node >& nodes);


void filter_ways_by_ranges(map< Uint31_Index, vector< Way_Skeleton > >& ways,
                           const set< pair< Uint31_Index, Uint31_Index > >& ranges);


class Way_Geometry_Store
{
public:
  Way_Geometry_Store(const map< Uint31_Index, vector< Way_Skeleton > >& ways,
                     const Statement& query, Resource_Manager& rman);
  
  // return the empty vector if the way is not found
  vector< Quad_Coord > get_geometry(const Way_Skeleton& way);
  
private:
  vector< Node > nodes;
};


//-----------------------------------------------------------------------------


template < typename TIndex, typename TObject >
void get_elements_by_id_from_db
    (map< TIndex, vector< TObject > >& elements,
     map< TIndex, vector< Attic< TObject > > >& attic_elements,
     const vector< typename TObject::Id_Type >& ids, bool invert_ids, uint64 timestamp,
     const set< pair< TIndex, TIndex > >& range_req,
     const Statement& query, Resource_Manager& rman,
     File_Properties& file_prop, File_Properties& attic_file_prop)
{
  elements.clear();
  attic_elements.clear();
  if (ids.empty())
  {
    collect_items_range(&query, rman, file_prop, range_req,
                        Trivial_Predicate< TObject >(), elements);
    if (timestamp != NOW)
    {
      collect_items_range(&query, rman, attic_file_prop, range_req,
                          Trivial_Predicate< Attic< TObject > >(), attic_elements);
      keep_only_least_younger_than(attic_elements, elements, timestamp);
    }
  }
  else if (!invert_ids)
  {
    //TODO
    collect_items_range(&query, rman, file_prop, range_req,
                        Id_Predicate< TObject >(ids), elements);
  }
  else if (!range_req.empty())
  {
    //TODO
    collect_items_range(&query, rman, file_prop, range_req,
                        Not_Predicate< TObject, Id_Predicate< TObject > >
                        (Id_Predicate< TObject >(ids)), elements);
  }
  else
  {
    //TODO
    collect_items_flat(query, rman, file_prop,
                        Not_Predicate< TObject, Id_Predicate< TObject > >
                        (Id_Predicate< TObject >(ids)), elements);
  }
}


template< typename Index, typename Skeleton >
void filter_attic_elements
    (Resource_Manager& rman, uint64 timestamp,
     std::map< Index, std::vector< Skeleton > >& current,
     std::map< Index, std::vector< Attic< Skeleton > > >& attic)
{
  if (timestamp != NOW)
  {
    std::vector< Index > idx_set;
    for (typename std::map< Index, std::vector< Skeleton > >::const_iterator it = current.begin();
         it != current.end(); ++it)
      idx_set.push_back(it->first);
    for (typename std::map< Index, std::vector< Attic< Skeleton > > >::const_iterator it = attic.begin();
         it != attic.end(); ++it)
      idx_set.push_back(it->first);
    std::sort(idx_set.begin(), idx_set.end());
    idx_set.erase(std::unique(idx_set.begin(), idx_set.end()), idx_set.end());
    
    // Remove elements that have been deleted at the given point of time
    std::map< Index, std::vector< typename Skeleton::Id_Type > > deleted_items;
    
    Block_Backend< Index, Attic< typename Skeleton::Id_Type >, typename std::vector< Index >::const_iterator >
        undeleted_db(rman.get_transaction()->data_index(attic_undeleted_file_properties< Skeleton >()));
    for (typename Block_Backend< Index, Attic< typename Skeleton::Id_Type >,
            typename std::vector< Index >::const_iterator >
        ::Discrete_Iterator
        it = undeleted_db.discrete_begin(idx_set.begin(), idx_set.end());
        !(it == undeleted_db.discrete_end()); ++it)
    {
      if (it.object().timestamp <= timestamp)
        continue;
      
      typename std::map< Index, vector< Skeleton > >::iterator cit = current.find(it.index());
      if (cit != current.end())
      {
        for (typename vector< Skeleton >::iterator it2 = cit->second.begin(); it2 != cit->second.end(); )
        {
          if (it2->id == it.object())
          {
            *it2 = cit->second.back();
            cit->second.pop_back();
          }
          else
            ++it2;
        }
      }
      
      typename std::map< Index, vector< Attic< Skeleton > > >::iterator ait = attic.find(it.index());
      if (ait != attic.end())
      {
        for (typename vector< Attic< Skeleton > >::iterator it2 = ait->second.begin();
             it2 != ait->second.end(); )
        {
          if (it2->id == it.object() && it.object().timestamp < it2->timestamp)
          {
            *it2 = ait->second.back();
            ait->second.pop_back();
          }
          else
            ++it2;
        }
      }
    }
    
    
    // Confirm elements that are backed by meta data
    // Update element's expiration timestamp if a meta exists that is older than the current
    // expiration date and younger than timestamp
    std::map< Index, std::map< typename Skeleton::Id_Type, std::pair< uint64, uint64 > > >
        timestamp_by_id_by_idx;
    for (typename std::map< Index, std::vector< Skeleton > >::const_iterator it = current.begin();
         it != current.end(); ++it)
    {
      std::map< typename Skeleton::Id_Type, std::pair< uint64, uint64 > >& entry
          = timestamp_by_id_by_idx[it->first];
      for (typename std::vector< Skeleton >::const_iterator it2 = it->second.begin();
           it2 != it->second.end(); ++it2)
        entry[it2->id] = std::make_pair(0, NOW);
    }
    for (typename std::map< Index, vector< Attic< Skeleton > > >::const_iterator it = attic.begin();
         it != attic.end(); ++it)
    {
      std::map< typename Skeleton::Id_Type, std::pair< uint64, uint64 > >& entry
          = timestamp_by_id_by_idx[it->first];
      for (typename std::vector< Attic< Skeleton > >::const_iterator it2 = it->second.begin();
           it2 != it->second.end(); ++it2)
        entry[it2->id] = std::make_pair(0, it2->timestamp);
    }
    
    Block_Backend< Index, OSM_Element_Metadata_Skeleton< typename Skeleton::Id_Type >,
            typename std::vector< Index >::const_iterator >
        attic_meta_db(rman.get_transaction()->data_index
          (attic_meta_file_properties< Skeleton >()));
    for (typename Block_Backend< Index, OSM_Element_Metadata_Skeleton< typename Skeleton::Id_Type >,
            typename std::vector< Index >::const_iterator >
        ::Discrete_Iterator
        it = attic_meta_db.discrete_begin(idx_set.begin(), idx_set.end());
        !(it == attic_meta_db.discrete_end()); ++it)
    {
      typename std::map< typename Skeleton::Id_Type, std::pair< uint64, uint64 > >::iterator
          tit = timestamp_by_id_by_idx[it.index()].find(it.object().ref);
      if (tit != timestamp_by_id_by_idx[it.index()].end())
      {
        if (timestamp < it.object().timestamp)
          tit->second.second = std::min(tit->second.second, it.object().timestamp);
        else
          tit->second.first = std::max(tit->second.first, it.object().timestamp);
      }
    }
    
    // Same thing with current meta data
    Block_Backend< Index, OSM_Element_Metadata_Skeleton< typename Skeleton::Id_Type >,
            typename std::vector< Index >::const_iterator >
        meta_db(rman.get_transaction()->data_index
          (current_meta_file_properties< Skeleton >()));
        
    for (typename Block_Backend< Index, OSM_Element_Metadata_Skeleton< typename Skeleton::Id_Type >,
            typename std::vector< Index >::const_iterator >
        ::Discrete_Iterator
        it = meta_db.discrete_begin(idx_set.begin(), idx_set.end());
        !(it == meta_db.discrete_end()); ++it)
    {
      typename std::map< typename Skeleton::Id_Type, std::pair< uint64, uint64 > >::iterator
          tit = timestamp_by_id_by_idx[it.index()].find(it.object().ref);
      if (tit != timestamp_by_id_by_idx[it.index()].end())
      {
        if (timestamp < it.object().timestamp)
          tit->second.second = std::min(tit->second.second, it.object().timestamp);
        else
          tit->second.first = std::max(tit->second.first, it.object().timestamp);
      }
    }
    
    // Filter current: only keep elements that have already existed at timestamp
    for (typename std::map< Index, std::vector< Skeleton > >::iterator it = current.begin();
         it != current.end(); ++it)
    {
      std::vector< Skeleton > result;      
      std::map< typename Skeleton::Id_Type, std::pair< uint64, uint64 > >& entry
          = timestamp_by_id_by_idx[it->first];
          
      for (typename std::vector< Skeleton >::const_iterator it2 = it->second.begin();
           it2 != it->second.end(); ++it2)
      {
        if (entry[it2->id].first > 0)
        {
          if (entry[it2->id].second == NOW)
            result.push_back(*it2);
          else
            attic[it->first].push_back(Attic< Skeleton >(*it2, entry[it2->id].second));
        }
      }
      
      result.swap(it->second);
    }
    
    // Filter attic: only keep elements that have already existed at timestamp
    for (typename std::map< Index, std::vector< Attic< Skeleton > > >::iterator it = attic.begin();
         it != attic.end(); ++it)
    {
      std::vector< Attic< Skeleton > > result;      
      std::map< typename Skeleton::Id_Type, std::pair< uint64, uint64 > >& entry
          = timestamp_by_id_by_idx[it->first];
          
      for (typename std::vector< Attic< Skeleton > >::const_iterator it2 = it->second.begin();
           it2 != it->second.end(); ++it2)
      {
        if (entry[it2->id].first > 0)
        {
          result.push_back(*it2);
          result.back().timestamp = entry[it2->id].second;
        }
      }
      
      result.swap(it->second);
    }
  }
}


#endif
