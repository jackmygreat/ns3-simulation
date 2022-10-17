/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Universita' di Firenze, Italy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Pavinberg (pavin0702@gmail.com)
 */

#ifndef DC_TOPOLOGY_H
#define DC_TOPOLOGY_H

#include "ns3/core-module.h"
#include "ns3/net-device.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
// #include "ns3/dpsk-module.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/topology.pb.h"

#include <_types/_uint32_t.h>
#include <vector>
#include <iterator>
#include <cstddef>

/**
 * \file
 * \ingroup protobuf-topology
 * ns3::DcTopology declaration
 */
namespace ns3 {

class DcTopology : public Object
{
public:
  /**
	 * \brief Get the type ID.
	 * \return the object TypeId.
	 */
  static TypeId GetTypeId (void);

  DcTopology () = default;
  DcTopology (uint32_t nodeNum);
  virtual ~DcTopology ();

  struct TopoNode
  {
    enum NodeType { HOST, SWITCH };
    NodeType type;
    Ptr<Node> nodePtr;

    const Ptr<Node>&
    operator-> () const
    {
      return nodePtr;
    }
  };

  void InstallNodes (ns3_proto::AllNodes nodes);

  void InstallNode (const uint32_t index, const TopoNode node);

  const TopoNode &GetNode (const uint32_t index) const;

  const Ptr<PointToPointNetDevice> GetNetDeviceOfNode (const uint32_t nodei, const uint32_t devi) const;

  const Ipv4InterfaceAddress GetInterfaceOfNode(const uint32_t nodei, uint32_t intfi) const;

private:
  std::vector<TopoNode> m_nodes;

public:
  /**
   * Iterators:
   * 1. use begin(), end() to iterate over all nodes
   * 2. use hosts_begin(), hosts_end() to iterate over hosts
   * 3. use switches_begin(), switches_end() to iterate over switches
   *
   * The definitions are at the end.
   */

  /* 1. Iterator */
  class Iterator
  {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = TopoNode;
    using pointer = TopoNode *; // or also value_type*
    using reference = TopoNode &; // or also value_type&

    Iterator (pointer ptr) : m_ptr (ptr)
    {
    }

    reference
    operator* () const
    {
      return *m_ptr;
    }
    
    pointer
    operator-> ()
    {
      return m_ptr;
    }

    // Prefix increment
    Iterator &
    operator++ ()
    {
      m_ptr++;
      return *this;
    }

    // Postfix increment
    Iterator
    operator++ (int)
    {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool
    operator== (const Iterator &lhs, const Iterator &rhs)
    {
      return lhs.m_ptr == rhs.m_ptr;
    }

    friend bool
    operator!= (const Iterator &lhs, const Iterator &rhs)
    {
      return lhs.m_ptr != rhs.m_ptr;
    }

  protected:
    pointer m_ptr;
  };
  
  Iterator
  begin ()
  {
    return Iterator (&m_nodes[0]);
  }
  
  Iterator
  end ()
  {
    return Iterator (&(*m_nodes.end ()));
  }
  
  class ConstIterator
  {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = TopoNode;
    using pointer = const TopoNode *; // or also value_type*
    using reference = const TopoNode &; // or also value_type&

    ConstIterator (pointer ptr) : m_ptr (ptr)
    {
    }

    reference
    operator* () const
    {
      return *m_ptr;
    }
    
    pointer
    operator-> () const
    {
      return m_ptr;
    }

    // Prefix increment
    ConstIterator &
    operator++ ()
    {
      m_ptr++;
      return *this;
    }

    // Postfix increment
    ConstIterator
    operator++ (int)
    {
      ConstIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool
    operator== (const ConstIterator &lhs, const ConstIterator &rhs)
    {
      return lhs.m_ptr == rhs.m_ptr;
    }

    friend bool
    operator!= (const ConstIterator &lhs, const ConstIterator &rhs)
    {
      return lhs.m_ptr != rhs.m_ptr;
    }

  protected:
    pointer m_ptr;
  };

  ConstIterator
  begin() const
  {
    return ConstIterator (&m_nodes[0]);
  }

  ConstIterator
  end () const
  {
    return ConstIterator (&(*m_nodes.end ()));
  }

  /* 2. HostIterator */
  class HostIterator final : public Iterator
  {
  public:
    using iterator_category = std::bidirectional_iterator_tag;

    HostIterator(pointer ptr, std::vector<TopoNode>::iterator end): Iterator(ptr), m_end(&(*end)) {}
    
    // Prefix increment
    HostIterator
    operator++ ()
    {
      while ((++m_ptr)->type != TopoNode::NodeType::HOST && m_ptr != m_end) {}
      return *this;
    }

    // Postfix increment
    HostIterator
    operator++ (int)
    {
      HostIterator tmp = *this;
      while ((++m_ptr)->type != TopoNode::NodeType::HOST && m_ptr != m_end) {}
      return tmp;
    }

  private:
    const pointer m_end;
  }; // struct HostIterator
  
  HostIterator
  hosts_begin ()
  {
    // return the first HOST
    for (TopoNode& node : m_nodes)
      {
        if (node.type == TopoNode::HOST) {
          return HostIterator (&node, m_nodes.end ());
        }
    }
    return HostIterator (&(*m_nodes.end ()), m_nodes.end ());
  }
  
  HostIterator
  hosts_end ()
  {
    return HostIterator (&(*m_nodes.end ()), m_nodes.end ());
  }

  /* 3. SwitchIterator */
  class SwitchIterator final : public Iterator
  {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    
    SwitchIterator (pointer ptr, const std::vector<TopoNode>::iterator end) : Iterator(ptr), m_end (&(*end))
    {
    }
    
    // Prefix increment
    SwitchIterator
    operator++ ()
    {
      while ((++m_ptr)->type != TopoNode::NodeType::SWITCH && m_ptr != m_end) {}
      return *this;
    }

    // Postfix increment
    SwitchIterator
    operator++ (int)
    {
      SwitchIterator tmp = *this;
      while ((++m_ptr)->type != TopoNode::NodeType::SWITCH && m_ptr != m_end) {}
      return tmp;
    }

  private:
    const pointer m_end;
  };
  
  SwitchIterator
  switches_begin ()
  {
    // return the first HOST
    for (TopoNode& node : m_nodes)
      {
        if (node.type == TopoNode::SWITCH) {
          return SwitchIterator (&node, m_nodes.end ());
        }
    }
    return SwitchIterator (&(*m_nodes.end ()), m_nodes.end ());
  }
  
  SwitchIterator
  switches_end ()
  {
    return SwitchIterator (&(*m_nodes.end ()), m_nodes.end ());
  }

}; // class DcTopology

} // namespace ns3

#endif
