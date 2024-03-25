use crate::{
    ast,
    infer::{BindingMap, NodeDescriptor},
};
use lang::{types, FragmentMap, NodeId};
use std::collections::HashMap;

#[derive(Clone, Debug, PartialEq)]
pub enum Inference<'a> {
    Reference(String),
    Property(NodeId, String),
    Arithmetic(NodeId, NodeId),
    FunctionResult(NodeId),
    Import(&'a ast::Import),
    Module(Vec<NodeId>),
    Parameter,
}

#[derive(Clone, Debug, PartialEq)]
pub enum Data<'a> {
    Infer(Inference<'a>),
    Inherit(NodeId),
    InheritKind(NodeId, types::Kind),
    Local(types::Type<NodeId>),
    // Remote(&'a types::ReferenceType<'a>),
}

pub type Weak<'a> = (types::Kind, Data<'a>);

pub type TypeMap<'a> = HashMap<NodeId, Weak<'a>>;

#[derive(Debug, PartialEq)]
pub struct Result<'a> {
    pub fragments: &'a FragmentMap,

    pub bindings: BindingMap,

    pub types: TypeMap<'a>,
}

impl<'a> Result<'a> {
    pub fn new(fragments: &'a FragmentMap) -> Self {
        Self {
            fragments,
            bindings: Default::default(),
            types: Default::default(),
        }
    }

    pub fn build_descriptors(&mut self) -> Vec<NodeDescriptor<'a>> {
        self.fragments
            .iter()
            .filter_map(|(id, (scope, ..))| {
                self.types.remove(id).map(|(kind, weak)| NodeDescriptor {
                    id: *id,
                    scope: scope.clone(),
                    kind,
                    weak,
                })
            })
            .collect()
    }
}
