mod component;
mod declaration;
mod expression;
mod import;
mod module;
mod parameter;
mod statement;
mod type_expression;

use crate::ast::{explode, walk};
use lang::types;

#[derive(Clone, Debug, PartialEq)]
pub enum Weak {
    Infer,
    Type(types::Type<walk::NodeId>),
    Inherit(walk::NodeId),
}

pub type WeakRef = (types::RefKind, Weak);

pub trait ToWeak {
    fn to_weak(&self) -> WeakRef;
}

impl ToWeak for explode::Fragment {
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
