use super::{
    walk::{self, NodeId},
    Storage,
};
use crate::Range;
use std::collections::{BTreeMap, HashMap};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ScopeId(pub Vec<usize>);

impl ScopeId {
    fn child(&self, next_id: usize) -> Self {
        Self([self.0.clone(), vec![next_id]].concat())
    }
}

impl Default for ScopeId {
    fn default() -> Self {
        Self(vec![0])
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum Fragment {
    Expression(super::Expression<NodeId, NodeId, NodeId>),
    Statement(super::Statement<NodeId>),
    Component(super::Component<NodeId, NodeId>),
    Parameter(super::Parameter<String, NodeId, NodeId>),
    Declaration(super::Declaration<String, NodeId, NodeId, NodeId, NodeId>),
    TypeExpression(super::TypeExpression<NodeId>),
    Import(super::Import),
    Module(super::Module<NodeId, NodeId>),
}

impl Fragment {
    pub fn to_binding(&self) -> Result<Vec<String>, ()> {
        match self {
            Self::Statement(super::Statement::Variable(binding, ..))
            | Self::Parameter(super::Parameter { binding, .. }) => Ok(vec![binding.clone()]),

            Self::Declaration(x) => Ok(vec![x.binding().clone()]),

            Self::Import(super::Import {
                path, alias: None, ..
            }) => Ok(vec![path.last().ok_or(())?.clone()]),

            Self::Import(super::Import {
                alias: Some(alias), ..
            }) => Ok(vec![alias.clone()]),

            _ => Ok(vec![]),
        }
    }
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct FragmentMap(pub BTreeMap<NodeId, (ScopeId, Fragment)>);

// impl FragmentMap {
//     pub fn to_descriptors(&self, mut weak_refs: HashMap<usize, WeakRef>) -> Vec<NodeDescriptor> {
//         self.0
//             .iter()
//             .filter_map(|(id, (scope, fragment))| match weak_refs.remove(id) {
//                 Some((kind, weak)) => Some(NodeDescriptor {
//                     id: *id,
//                     kind,
//                     scope: scope.clone(),
//                     fragment: fragment.clone(),
//                     weak,
//                 }),
//                 _ => None,
//             })
//             .collect()
//     }
// }

impl FromIterator<(NodeId, (ScopeId, Fragment))> for FragmentMap {
    fn from_iter<T: IntoIterator<Item = (NodeId, (ScopeId, Fragment))>>(iter: T) -> Self {
        Self(BTreeMap::from_iter(iter))
    }
}

pub trait Explode {
    fn explode(self) -> FragmentMap;
}

#[derive(Default)]
struct State {
    next_node_id: usize,
    next_scope_id: usize,
    pub fragments: FragmentMap,
}

#[derive(Default)]
pub struct Visitor {
    scope_id: ScopeId,
    state: State,
}

impl Visitor {
    fn next_node_id(&mut self) -> usize {
        let id = self.state.next_node_id;
        self.state.next_node_id += 1;
        id
    }

    fn next_scope_id(&mut self) -> usize {
        let id = self.state.next_scope_id;
        self.state.next_scope_id += 1;
        id
    }

    fn scope<T, F>(mut self, f: F) -> (T, Self)
    where
        F: FnOnce(Self) -> (T, Self),
    {
        let scope_id = self.next_scope_id();
        let visitor = Self {
            scope_id: self.scope_id.child(scope_id),
            state: self.state,
        };

        let (result, Self { state, .. }) = f(visitor);

        (
            result,
            Self {
                scope_id: self.scope_id,
                state,
            },
        )
    }

    pub fn capture(mut self, fragment: Fragment) -> (NodeId, Self) {
        let node_id = NodeId(self.next_node_id());
        let scope_id = self.scope_id.clone();
        self.state.fragments.0.insert(node_id, (scope_id, fragment));
        (node_id, self)
    }

    pub fn fragments(self) -> FragmentMap {
        self.state.fragments
    }
}

impl walk::Visit for Visitor {
    type Binding = String;
    type Expression = NodeId;
    type Statement = NodeId;
    type Component = NodeId;
    type TypeExpression = NodeId;
    type Parameter = NodeId;
    type Declaration = NodeId;
    type Import = NodeId;
    type Module = NodeId;

    fn binding(self, x: super::Binding, _: Range) -> (Self::Binding, Self) {
        (x.0, self)
    }

    fn expression(
        self,
        x: super::Expression<Self::Expression, Self::Statement, Self::Component>,
        _: Range,
    ) -> (Self::Expression, Self) {
        self.capture(Fragment::Expression(x))
    }

    fn statement(self, x: super::Statement<Self::Expression>, _: Range) -> (Self::Statement, Self) {
        self.capture(Fragment::Statement(x))
    }

    fn component(
        self,
        x: super::Component<Self::Expression, Self::Component>,
        _: Range,
    ) -> (Self::Component, Self) {
        self.capture(Fragment::Component(x))
    }

    fn type_expression(
        self,
        x: super::TypeExpression<Self::TypeExpression>,
        _: Range,
    ) -> (Self::TypeExpression, Self) {
        self.capture(Fragment::TypeExpression(x))
    }

    fn parameter(
        self,
        x: super::Parameter<String, Self::Expression, Self::TypeExpression>,
        _: Range,
    ) -> (Self::Parameter, Self) {
        self.capture(Fragment::Parameter(x))
    }

    fn declaration(
        self,
        x: super::Declaration<
            String,
            Self::Expression,
            Self::Parameter,
            Self::Module,
            Self::TypeExpression,
        >,
        _: Range,
    ) -> (Self::Declaration, Self) {
        self.capture(Fragment::Declaration(x))
    }

    fn import(self, x: super::Import, _: Range) -> (Self::Import, Self) {
        self.capture(Fragment::Import(x))
    }

    fn module(
        self,
        x: super::Module<Self::Import, Self::Declaration>,
        _: Range,
    ) -> (Self::Module, Self) {
        self.capture(Fragment::Module(x))
    }
}

// impl super::Program {
//     fn collect(self) -> Vec<(NodeId, Fragment)> {
//         self.0.walk(Visitor::default()).1.into_fragments()
//     }
// }

#[cfg(test)]
mod tests {
    use super::Fragment;
    use crate::{
        ast::{
            self,
            explode::{Explode, FragmentMap, ScopeId},
            walk::NodeId,
        },
        test::{fixture, mock},
    };
    use kore::assert_eq;

    #[test]
    fn collect() {
        let program = mock::Module::new(ast::Module::new(
            vec![fixture::import::mock()],
            vec![fixture::type_alias::mock()],
        ));

        assert_eq!(
            program.explode(),
            FragmentMap::from_iter(
                [
                    fixture::import::fragments(0, (vec![0], 0)),
                    fixture::type_alias::fragments(1, (vec![0], 1)),
                    fixture::constant::fragments(3, (vec![0], 2)),
                    fixture::enumerated::fragments(6, (vec![0], 3)),
                    fixture::function::fragments(8, (vec![0], 4)),
                    fixture::view::fragments(14, (vec![0], 5)),
                    fixture::module::fragments(19, (vec![0], 6)),
                    vec![(
                        NodeId(24),
                        (
                            ScopeId(vec![0]),
                            Fragment::Module(ast::Module {
                                imports: vec![NodeId(0)],
                                declarations: vec![
                                    NodeId(2),
                                    NodeId(5),
                                    NodeId(7),
                                    NodeId(13),
                                    NodeId(18),
                                    NodeId(23),
                                ]
                            })
                        )
                    )]
                ]
                .concat()
            )
        );
    }
}
