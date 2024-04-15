use std::marker::PhantomData;

use crate::{
    walk::{CommonVisitor, ProgramVisitor, Walk},
    Fragment, FragmentMap, NodeId, Range, ScopeId,
};
use kore::Incrementor;

pub trait IntoFragments<T> {
    fn into_fragments(self) -> FragmentMap<T>;
}

impl<Context> super::into_fragments::IntoFragments<NodeId> for super::meta::Program<Context> {
    fn into_fragments(self) -> FragmentMap<NodeId> {
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
    pub fragments: FragmentMap<NodeId>,
}

pub struct Visitor<Context> {
    _context: PhantomData<Context>,
    scope_id: ScopeId,
    state: State,
}

impl<Context> Default for Visitor<Context> {
    fn default() -> Self {
        let mut state = State::default();
        Self {
            _context: PhantomData,
            scope_id: ScopeId(vec![state.scope_id.increment()]),
            state,
        }
    }
}

impl<Context> Visitor<Context> {
    const fn new(scope_id: ScopeId, state: State) -> Self {
        Self {
            _context: PhantomData,
            scope_id,
            state,
        }
    }

    fn next_scope_id(&mut self) -> ScopeId {
        self.scope_id.child(self.state.scope_id.increment())
    }

    fn capture(mut self, fragment: Fragment) -> (NodeId, Self) {
        let node_id = NodeId(self.state.node_id.increment());
        let scope_id = self.scope_id.clone();
        self.state.fragments.insert(node_id, (scope_id, fragment));
        (node_id, self)
    }

    fn fragments(self) -> FragmentMap<NodeId> {
        self.state.fragments
    }
}

impl<Context> CommonVisitor for Visitor<Context> {
    type Context = Context;
    type Binding = String;
    type TypeExpression = NodeId;

    fn scoped<T, F>(mut self, f: F) -> (T, Self)
    where
        F: FnOnce(Self) -> (T, Self),
    {
        let child = Self::new(self.next_scope_id(), self.state);

        let (result, Self { state, .. }) = f(child);

        (result, Self::new(self.scope_id, state))
    }

    fn binding(self, x: super::Binding, _: Range) -> (Self::Binding, Self) {
        (x.0, self)
    }

    fn type_expression(
        self,
        x: super::TypeExpression<Self::Binding, Self::TypeExpression>,
        _: Self::Context,
    ) -> (Self::TypeExpression, Self) {
        self.capture(Fragment::TypeExpression(x))
    }
}

impl<Context> ProgramVisitor for Visitor<Context> {
    type Expression = NodeId;
    type Statement = NodeId;
    type Attribute = NodeId;
    type Component = NodeId;
    type Parameter = NodeId;
    type Declaration = NodeId;
    type Import = NodeId;
    type Module = NodeId;

    fn expression(
        self,
        x: super::Expression<Self::Expression, Self::Statement, Self::Component>,
        _: Self::Context,
    ) -> (Self::Expression, Self) {
        self.capture(Fragment::Expression(x))
    }

    fn statement(
        self,
        x: super::Statement<Self::Expression>,
        _: Self::Context,
    ) -> (Self::Statement, Self) {
        self.capture(Fragment::Statement(x))
    }

    fn attribute(
        self,
        x: super::Attribute<Self::Expression>,
        _: Self::Context,
    ) -> (Self::Attribute, Self) {
        self.capture(Fragment::Attribute(x))
    }

    fn component(
        self,
        x: super::Component<Self::Expression, Self::Component, Self::Attribute>,
        _: Self::Context,
    ) -> (Self::Component, Self) {
        self.capture(Fragment::Component(x))
    }

    fn parameter(
        self,
        x: super::Parameter<String, Self::Expression, Self::TypeExpression>,
        _: Self::Context,
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
        _: Self::Context,
    ) -> (Self::Declaration, Self) {
        self.capture(Fragment::Declaration(x))
    }

    fn import(self, x: super::Import, _: Self::Context) -> (Self::Import, Self) {
        self.capture(Fragment::Import(x))
    }

    fn module(
        self,
        x: super::Module<Self::Import, Self::Declaration>,
        _: Self::Context,
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
                    fixture::import::fragments_at(0, &(vec![0], 0)),
                    fixture::type_alias::fragments_at(1, &(vec![0], 0)),
                    fixture::constant::fragments_at(3, &(vec![0], 1)),
                    fixture::enumerated::fragments_at(6, &(vec![0], 2)),
                    fixture::function::fragments_at(9, &(vec![0], 3)),
                    fixture::view::fragments_at(21, &(vec![0], 4)),
                    fixture::module::fragments_at(42, &(vec![0], 6)),
                    vec![(
                        NodeId(50),
                        (
                            ScopeId(vec![0]),
                            Fragment::Module(ast::Module {
                                imports: vec![NodeId(0)],
                                declarations: vec![
                                    NodeId(2),
                                    NodeId(5),
                                    NodeId(8),
                                    NodeId(20),
                                    NodeId(41),
                                    NodeId(49),
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
