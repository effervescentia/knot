mod component;
mod declaration;
mod expression;
mod import;
mod module;
mod parameter;
mod statement;
mod type_expression;

use crate::{
    ast,
    data::{DeconstructedModule, NodeDescriptor},
};
use lang::{types, Fragment, FragmentMap, NodeId};
use std::collections::HashMap;

#[derive(Clone, Debug, PartialEq)]
pub enum Inference<'a> {
    Resolve(String),
    Property(NodeId, String),
    Arithmetic(NodeId, NodeId),
    FunctionResult(NodeId),
    Import(&'a ast::Import),
    Module(Vec<NodeId>),
    Parameter,
}

#[derive(Clone, Debug, PartialEq)]
pub enum Type<'a> {
    Infer(Inference<'a>),
    Inherit(NodeId),
    InheritKind(NodeId, types::RefKind),
    Local(types::Type<NodeId>),
    Remote(&'a types::ReferenceType<'a>),
}

impl<'a> Type<'a> {
    pub fn inherit_from_type(id: NodeId) -> Self {
        Self::InheritKind(id, types::RefKind::Type)
    }

    // pub fn resolve(
    //     &self,
    //     id: &NodeId,
    //     allowed_kind: &types::RefKind,
    // ) -> Option<std::result::Result<&types::ReferenceType, ()>> {
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

pub type Ref<'a> = (types::RefKind, Type<'a>);

pub trait ToWeak {
    fn to_weak(&self) -> Ref;
}

impl ToWeak for Fragment {
    fn to_weak(&self) -> Ref {
        match self {
            Self::Expression(x) => x.to_weak(),
            Self::Statement(x) => x.to_weak(),
            Self::Component(x) => x.to_weak(),
            Self::Parameter(x) => x.to_weak(),
            Self::TypeExpression(x) => x.to_weak(),
            Self::Declaration(x) => x.to_weak(),
            Self::Import(x) => x.to_weak(),
            Self::Module(x) => x.to_weak(),
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct Result<'a> {
    pub module: DeconstructedModule,

    pub refs: HashMap<NodeId, Ref<'a>>,
}

impl<'a> Result<'a> {
    pub fn new(fragments: FragmentMap) -> Self {
        Self {
            module: DeconstructedModule::from_fragments(fragments),
            refs: HashMap::default(),
        }
    }

    pub fn to_descriptors(&mut self) -> Vec<NodeDescriptor> {
        self.module
            .fragments
            .0
            .iter()
            .filter_map(|(id, (scope, fragment))| match self.refs.remove(id) {
                Some((kind, weak)) => Some(NodeDescriptor {
                    id: *id,
                    kind,
                    scope: scope.clone(),
                    fragment: fragment.clone(),
                    weak,
                }),
                _ => None,
            })
            .collect()
    }
}
