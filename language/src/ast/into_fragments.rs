use super::walk::{self, Walk};
use crate::{Fragment, FragmentMap, NodeId, Range, ScopeId};
use kore::Incrementor;

pub trait IntoFragments {
    fn into_fragments(self) -> FragmentMap;
}

impl<Context> super::into_fragments::IntoFragments for super::meta::Program<Context> {
    fn into_fragments(self) -> FragmentMap {
        self.0
            .walk(super::into_fragments::Visitor::default())
            .1
            .fragments()
    }
}

#[derive(Default)]
struct State {
    node_id: Incrementor,
    scope_id: Incrementor,
    pub fragments: FragmentMap,
}

pub struct Visitor {
    scope_id: ScopeId,
    state: State,
}

impl Default for Visitor {
    fn default() -> Self {
        let mut state = State::default();
        Self {
            scope_id: ScopeId(vec![state.scope_id.increment()]),
            state,
        }
    }
}

impl Visitor {
    fn next_scope_id(&mut self) -> ScopeId {
        self.scope_id.child(self.state.scope_id.increment())
    }

    pub fn capture(mut self, fragment: Fragment) -> (NodeId, Self) {
        let node_id = NodeId(self.state.node_id.increment());
        let scope_id = self.scope_id.clone();
        self.state.fragments.insert(node_id, (scope_id, fragment));
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

    fn scoped<T, F>(mut self, f: F) -> (T, Self)
    where
        F: FnOnce(Self) -> (T, Self),
    {
        let child = Self {
            scope_id: self.next_scope_id(),
            state: self.state,
        };

        let (result, Self { state, .. }) = f(child);

        (
            result,
            Self {
                scope_id: self.scope_id,
                state,
            },
        )
    }

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

#[cfg(test)]
mod tests {
    use super::Fragment;
    use crate::{
        ast::{
            self,
            into_fragments::{FragmentMap, IntoFragments, ScopeId},
        },
        test::fixture,
        NodeId,
    };
    use kore::assert_eq;

    #[test]
    fn collect() {
        let program = ast::meta::Program(ast::meta::Module::mock(ast::Module::new(
            vec![fixture::import::mock()],
            vec![
                fixture::type_alias::mock(),
                fixture::constant::mock(),
                fixture::enumerated::mock(),
                fixture::function::mock(),
                fixture::view::mock(),
                fixture::module::mock(),
            ],
        )));

        assert_eq!(
            program.into_fragments(),
            FragmentMap::from_iter(
                [
                    fixture::import::fragments(0, &(vec![0], 0)),
                    fixture::type_alias::fragments(1, &(vec![0], 0)),
                    fixture::constant::fragments(3, &(vec![0], 1)),
                    fixture::enumerated::fragments(6, &(vec![0], 2)),
                    fixture::function::fragments(8, &(vec![0], 3)),
                    fixture::view::fragments(14, &(vec![0], 4)),
                    fixture::module::fragments(19, &(vec![0], 5)),
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
