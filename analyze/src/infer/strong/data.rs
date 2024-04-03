use crate::error::Error;
use lang::{ast, types, CanonicalId, NodeId};
use std::{cell::OnceCell, collections::HashMap, rc::Rc};

/// the inferred type for nodes in a strongly typed AST
#[derive(Clone, Debug, PartialEq)]
pub enum Type {
    Inherit(CanonicalId),
    Value(types::Type<CanonicalId>),
}

pub type Strong = (types::Kind, Result<Type, Error>);

/// output of the strong inference phase
#[derive(Debug, PartialEq)]
pub struct Output {
    /// lookup for strong types during inference
    pub types: HashMap<NodeId, OnceCell<Rc<(CanonicalId, ast::typed::Type)>>>,
}

impl Output {
    pub fn new<'a, Iterable>(keys: Iterable) -> Self
    where
        Iterable: IntoIterator<Item = &'a NodeId>,
    {
        Self {
            types: keys.into_iter().map(|id| (*id, OnceCell::new())).collect(),
        }
    }
}

#[derive(Debug, PartialEq)]
pub enum Action {
    /// try to infer the type on the next pass
    Skip,

    /// infer a success result based on the data
    Infer(Type),

    /// used when inheriting can resolve further
    InheritAndSkip(CanonicalId),

    /// infer an error result based on the data
    Raise(Error),
}
