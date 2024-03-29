use crate::error::ResolveError;
use lang::{ast, types, NodeId};
use std::{cell::OnceCell, collections::HashMap, rc::Rc};

#[derive(Clone, Debug, PartialEq)]
pub enum Data {
    Inherit(NodeId),
    Local(types::Type<NodeId>),
    // Remote(&'a types::ReferenceType<'a>),
}

pub type Strong = (types::Kind, std::result::Result<Data, ResolveError>);

#[derive(Debug, PartialEq)]
pub struct Output {
    // might be able to refactor this to avoid the need for
    // inherit types thanks to Rc container
    pub types: HashMap<NodeId, OnceCell<Rc<ast::typed::Type>>>,

    pub inherits: HashMap<NodeId, NodeId>,
}

impl Output {
    pub fn new<'a, Iterable>(keys: Iterable) -> Self
    where
        Iterable: IntoIterator<Item = &'a NodeId>,
    {
        Self {
            types: keys.into_iter().map(|id| (*id, OnceCell::new())).collect(),
            inherits: Default::default(),
        }
    }
}

pub type Result = crate::Result<Output>;

#[derive(Debug, PartialEq)]
pub enum Action {
    /// try to infer the type on the next pass
    Skip,

    /// infer a success result based on the data
    Infer(Data),

    /// used when inheriting can resolve further
    InheritAndSkip(NodeId),

    /// infer an error result based on the data
    Raise(ResolveError),
}
