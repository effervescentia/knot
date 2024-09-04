use crate::{error::Error, Context, TypeMap};
use kore::invariant;
use lang::{ast, types, CanonicalId, Canonicalize, NodeId};
use std::{cell::OnceCell, collections::HashMap, fmt::Debug, rc::Rc};

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
    pub types: HashMap<NodeId, OnceCell<Rc<ast::typed::Meta>>>,
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

    pub fn canonicalize(&self, ctx: &Context) -> TypeMap {
        self.types
            .iter()
            .map(|(key, value)| {
                (
                    ctx.canonicalize(*key),
                    value
                        .get()
                        .map(Rc::clone)
                        .unwrap_or_else(|| invariant!("all cells should be populated")),
                )
            })
            .collect()
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
