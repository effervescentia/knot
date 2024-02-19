use crate::{
    data::{DeconstructedModule, ScopedType},
    error::ResolveError,
    strong,
};
use lang::{types, NodeId};
use std::collections::HashMap;

#[derive(Clone, Debug, PartialEq)]
pub struct NotInferrable<'a>(Vec<&'a NodeId>);

pub type Type<'a> = std::result::Result<ScopedType<'a>, ResolveError>;

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

    pub fn get_type(&self, id: &NodeId, allowed_kind: &types::RefKind) -> Option<&strong::Type> {
        self.refs
            .get(id)
            .and_then(|(kind, strong)| allowed_kind.can_accept(kind).then_some(strong))
    }

    pub fn resolve_type(
        &self,
        id: &NodeId,
        allowed_kind: &types::RefKind,
    ) -> Option<std::result::Result<&types::Type<NodeId>, ()>> {
        match self.get_type(id, allowed_kind) {
            Some(Ok(ScopedType::Inherit(x))) => self.resolve_type(x, allowed_kind),

            Some(Ok(ScopedType::InheritKind(x, from_kind))) => self.resolve_type(x, from_kind),

            Some(Ok(ScopedType::Type(x))) => Some(Ok(&x)),

            Some(Ok(ScopedType::External(x))) => unimplemented!(),

            Some(Err(_)) => Some(Err(())),

            None => None,
        }
    }
}
