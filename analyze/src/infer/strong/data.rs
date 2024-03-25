use crate::{error::ResolveError, typed};
use lang::{
    types::{self},
    NodeId,
};
use std::{cell::OnceCell, collections::HashMap, rc::Rc};

#[derive(Clone, Debug, PartialEq)]
pub enum Data {
    Inherit(NodeId),
    Local(types::Type<NodeId>),
    // Remote(&'a types::ReferenceType<'a>),
}

pub type Strong = (types::Kind, std::result::Result<Data, ResolveError>);

pub struct Output {
    pub types: HashMap<NodeId, OnceCell<Rc<typed::ReferenceType>>>,

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
    Skip,
    Infer(Data),
    Inherit(NodeId),
    Raise(ResolveError),
}
