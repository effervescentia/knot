use crate::{
    data::{DeconstructedModule, ScopedType},
    error::SemanticError,
    strong,
};
// use kore::invariant;
use lang::{types, NodeId};
use std::collections::HashMap;

pub type Type<'a> = std::result::Result<ScopedType<'a>, SemanticError>;

pub type Ref<'a> = (types::RefKind, Type<'a>);

#[derive(Clone, Debug, PartialEq)]
pub struct Result<'a> {
    pub module: DeconstructedModule,

    pub refs: HashMap<NodeId, Ref<'a>>,
}

impl<'a> Result<'a> {
    pub fn new(module: DeconstructedModule) -> Self {
        Self {
            module,
            refs: HashMap::default(),
        }
    }

    pub fn get_shallow_type(
        &self,
        id: &NodeId,
        allowed_kind: &types::RefKind,
    ) -> Option<&strong::Type> {
        self.refs
            .get(id)
            .and_then(|(kind, strong)| allowed_kind.can_accept(kind).then_some(strong))
    }

    pub fn get_type_shape(
        &self,
        id: &NodeId,
        allowed_kind: &types::RefKind,
    ) -> Option<std::result::Result<&types::TypeShape, ()>> {
        match self.get_shallow_type(id, allowed_kind) {
            Some(Ok(ScopedType::Inherit(x))) => self.get_type_shape(x, allowed_kind),

            Some(Ok(ScopedType::InheritKind(x, from_kind))) => self.get_type_shape(x, from_kind),

            Some(Ok(ScopedType::Type(x))) => Some(Ok(&x.to_shape())),

            Some(Ok(ScopedType::External(x))) => Some(Ok(x)),

            Some(Err(_)) => Some(Err(())),

            None => None,
        }
    }

    // pub fn as_strong<'b>(&'b self, id: &'b NodeId, kind: &'b types::RefKind) -> Option<&'b Strong> {
    //     self.refs.get(id).and_then(|(found_kind, strong)| {
    //         (found_kind == kind || found_kind == &types::RefKind::Mixed).then_some(strong)
    //     })
    // }

    // pub fn resolve(&self, id: &NodeId) -> &Strong {
    //     match self.refs.get(id) {
    //         Some((_, x)) => x,

    //         None => invariant!("all nodes should have a corresponding strong ref"),
    //     }
    // }

    // pub fn inherit_as(
    //     &mut self,
    //     (from_id, from_kind): (NodeId, &types::RefKind),
    //     (to_id, to_kind): (NodeId, &types::RefKind),
    // ) -> bool {
    //     if let Some(strong) = self.as_strong(&from_id, from_kind) {
    //         self.refs.insert(to_id, (*to_kind, strong.clone()));
    //         true
    //     } else {
    //         false
    //     }
    // }

    // pub fn inherit(&mut self, node: &NodeDescriptor, from_id: NodeId) -> bool {
    //     self.inherit_as((from_id, &node.kind), (node.id, &node.kind))
    // }
}
