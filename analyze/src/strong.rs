use crate::{data::DeconstructedModule, error::ResolveError, strong};
use lang::{types, NodeId};
use std::collections::HashMap;

pub type Type<'a> = std::result::Result<&'a types::ReferenceType<'a>, ResolveError>;

pub type Ref<'a> = (types::RefKind, Type<'a>);

#[derive(Clone, Debug, PartialEq)]
pub struct State<'a> {
    pub module: DeconstructedModule,

    pub refs: HashMap<NodeId, Ref<'a>>,
}

impl<'a> State<'a> {
    pub fn new(module: DeconstructedModule) -> Self {
        Self {
            module,
            refs: HashMap::default(),
        }
    }

    pub fn get_type(&self, id: &NodeId, allowed_kind: &types::RefKind) -> Option<&strong::Type> {
        self.refs
            .get(id)
            .and_then(|(kind, strong)| allowed_kind.can_accept(kind).then_some(strong))
    }

    pub fn resolve_type(
        &self,
        id: &NodeId,
        allowed_kind: &types::RefKind,
    ) -> Option<std::result::Result<&types::ReferenceType, ResolveError>> {
        match self.get_type(id, allowed_kind) {
            Some(Ok(x)) => Some(Ok(x)),

            Some(Err(_)) => Some(Err(ResolveError::NotInferrable(vec![*id]))),

            None => None,
        }
    }

    // pub fn resolve_type(
    //     &self,
    //     id: &NodeId,
    //     allowed_kind: &types::RefKind,
    // ) -> Option<std::result::Result<&types::Type<NodeId>, ()>> {
    //     match self.get_type(id, allowed_kind) {
    //         Some(Ok(ScopedType::Inherit(x))) => self.resolve_type(x, allowed_kind),

    //         Some(Ok(ScopedType::InheritKind(x, from_kind))) => self.resolve_type(x, from_kind),

    //         Some(Ok(ScopedType::Type(x))) => Some(Ok(&x)),

    //         Some(Ok(ScopedType::External(x))) => unimplemented!(),

    //         Some(Err(_)) => Some(Err(())),

    //         None => None,
    //     }
    // }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Result<'a> {
    pub module: DeconstructedModule,

    pub refs: HashMap<NodeId, Type<'a>>,
}
