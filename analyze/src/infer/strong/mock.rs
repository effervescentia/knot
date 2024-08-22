use super::{data::Type, state::State};
use crate::{error::Error, Context};
use kore::Serializable;
use lang::{types::Kind, Fragment, NodeId, ScopeId};
use std::collections::BTreeMap;

pub const FRAGMENTS: &BTreeMap<NodeId, (ScopeId, Fragment)> = &BTreeMap::new();

impl<'a, Library> State<'a, Library>
where
    Library: Serializable,
{
    #[allow(clippy::type_complexity)]
    pub fn mock(context: &'a Context<Library>) -> Self {
        State {
            context,
            fragments: FRAGMENTS,
            bindings: Default::default(),
            nodes: vec![],
            types: Default::default(),
            warnings: vec![],
        }
    }

    #[allow(clippy::type_complexity)]
    pub fn from_types(
        context: &'a Context<Library>,
        types: Vec<(NodeId, (Kind, Result<Type, Error>))>,
    ) -> Self {
        State {
            types: BTreeMap::from_iter(types),
            ..Self::mock(context)
        }
    }
}
