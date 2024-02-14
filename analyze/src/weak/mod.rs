mod component;
mod declaration;
mod expression;
mod import;
mod module;
mod parameter;
mod statement;
mod type_expression;

use crate::context::ProgramContext;
use lang::{types, Fragment, FragmentMap, NodeId};
use std::collections::HashMap;

#[derive(Clone, Debug, PartialEq)]
pub enum Weak {
    Infer,
    Type(types::Type<NodeId>),
    Inherit(NodeId),
}

pub type WeakRef = (types::RefKind, Weak);

pub trait ToWeak {
    fn to_weak(&self) -> WeakRef;
}

impl ToWeak for Fragment {
    fn to_weak(&self) -> WeakRef {
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
pub struct WeakResult {
    pub program: ProgramContext,

    pub refs: HashMap<NodeId, WeakRef>,
}

impl WeakResult {
    pub fn new(fragments: FragmentMap) -> Self {
        Self {
            program: ProgramContext::from_fragments(fragments),
            refs: HashMap::default(),
        }
    }
}
