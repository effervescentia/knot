pub use lang::ast::*;

pub mod typed {
    use std::rc::Rc;

    use lang::{types, Node, Range};

    #[derive(Clone, Debug, PartialEq)]
    pub struct ReferenceType(pub types::Type<Rc<ReferenceType>>);

    type Typed<Value> = Node<Value, ReferenceType>;

    #[derive(Debug, PartialEq)]
    pub struct Binding(pub Node<super::Binding, ()>);

    impl Binding {
        pub const fn new(x: super::Binding, range: Range) -> Self {
            Self(Node::raw(x, range))
        }
    }

    #[derive(Debug, PartialEq)]
    pub struct Expression(pub Typed<super::Expression<Expression, Statement, Component>>);

    #[derive(Debug, PartialEq)]
    pub struct Statement(pub Typed<super::Statement<Expression>>);

    #[derive(Debug, PartialEq)]
    pub struct Component(pub Typed<super::Component<Component, Expression>>);

    #[derive(Debug, PartialEq)]
    pub struct TypeExpression(pub Typed<super::TypeExpression<TypeExpression>>);

    #[derive(Debug, PartialEq)]
    pub struct Parameter(pub Typed<super::Parameter<Binding, Expression, TypeExpression>>);

    #[derive(Debug, PartialEq)]
    pub struct Declaration(
        pub Typed<super::Declaration<Binding, Expression, TypeExpression, Parameter, Module>>,
    );

    #[derive(Debug, PartialEq)]
    pub struct Import(pub Typed<super::Import>);

    #[derive(Debug, PartialEq)]
    pub struct Module(pub Typed<super::Module<Import, Declaration>>);

    #[derive(Debug, PartialEq)]
    pub struct Program(pub Module);
}
