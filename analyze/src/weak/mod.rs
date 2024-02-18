mod component;
mod declaration;
mod expression;
mod import;
mod module;
mod parameter;
mod statement;
mod type_expression;

use crate::data::{DeconstructedModule, NodeDescriptor, ScopedType};
use lang::{types, Fragment, FragmentMap, NodeId};
use std::collections::HashMap;

pub type Type<'a> = Option<ScopedType<'a>>;

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

    pub fn to_descriptors(&self) -> Vec<NodeDescriptor> {
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
