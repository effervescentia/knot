mod component;
mod declaration;
mod expression;
mod import;
mod module;
mod parameter;
mod statement;
mod type_expression;

use crate::data::{ModuleMetadata, ScopedType};
use lang::{types, Fragment, FragmentMap, NodeId};
use std::collections::HashMap;

pub type Weak<'a> = Option<ScopedType<'a>>;

pub type WeakRef<'a> = (types::RefKind, Weak<'a>);

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
pub struct WeakResult<'a> {
    pub module: ModuleMetadata,

    pub refs: HashMap<NodeId, WeakRef<'a>>,
}

impl<'a> WeakResult<'a> {
    pub fn new(fragments: FragmentMap) -> Self {
        Self {
            module: ModuleMetadata::from_fragments(fragments),
            refs: HashMap::default(),
        }
    }
}
