use super::{data::Data, state::State};
use crate::{error::ResolveError, infer::BindingMap};
use lang::{types::Kind, Fragment, NodeId, ScopeId};
use std::collections::{BTreeMap, HashMap};

pub const FRAGMENTS: &BTreeMap<NodeId, (ScopeId, Fragment)> = &BTreeMap::new();

impl<'a> State<'a> {
    #[allow(clippy::type_complexity)]
    pub fn from_types(types: Vec<(NodeId, (Kind, Result<Data, ResolveError>))>) -> State<'static> {
        State {
            fragments: FRAGMENTS,
            bindings: BindingMap(HashMap::new()),
            nodes: vec![],
            types: BTreeMap::from_iter(types),
            warnings: vec![],
        }
    }
}
