use super::{data::Type, state::State};
use crate::{error::ResolveError, Context};
use lang::{types::Kind, Fragment, NodeId, ScopeId};
use std::collections::BTreeMap;

pub const FRAGMENTS: &BTreeMap<NodeId, (ScopeId, Fragment)> = &BTreeMap::new();

impl<'a> State<'a> {
    #[allow(clippy::type_complexity)]
    pub fn mock(context: &'a Context) -> State<'a> {
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
        context: &'a Context,
        types: Vec<(NodeId, (Kind, Result<Type, ResolveError>))>,
    ) -> State<'a> {
        State {
            types: BTreeMap::from_iter(types),
            ..Self::mock(context)
        }
    }
}
