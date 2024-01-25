pub use lang::ast::*;

pub mod raw {
    use lang::{Node, Range};

    type Raw<T> = Node<T, ()>;

    #[derive(Debug, PartialEq)]
    pub struct Binding(pub Raw<super::Binding>);

    impl Binding {
        pub const fn new(x: super::Binding, range: Range) -> Self {
            Self(Node::raw(x, range))
        }
    }

    pub type Storage = super::Storage<Binding>;

    #[derive(Debug, PartialEq)]
    pub struct Expression(pub Raw<super::Expression<Expression, Statement, Component>>);

    impl Expression {
        pub const fn new(x: super::Expression<Self, Statement, Component>, range: Range) -> Self {
            Self(Node::raw(x, range))
        }
    }

    #[derive(Debug, PartialEq)]
    pub struct Statement(pub Raw<super::Statement<Expression>>);

    impl Statement {
        pub const fn new(x: super::Statement<Expression>, range: Range) -> Self {
            Self(Node::raw(x, range))
        }
    }

    #[derive(Debug, PartialEq)]
    pub struct Component(pub Raw<super::Component<Component, Expression>>);

    impl Component {
        pub const fn new(x: super::Component<Self, Expression>, range: Range) -> Self {
            Self(Node::raw(x, range))
        }
    }

    #[derive(Debug, PartialEq)]
    pub struct TypeExpression(pub Raw<super::TypeExpression<TypeExpression>>);

    impl TypeExpression {
        pub const fn new(x: super::TypeExpression<Self>, range: Range) -> Self {
            Self(Node::raw(x, range))
        }
    }

    #[derive(Debug, PartialEq)]
    pub struct Parameter(pub Raw<super::Parameter<Binding, Expression, TypeExpression>>);

    impl Parameter {
        pub const fn new(
            x: super::Parameter<Binding, Expression, TypeExpression>,
            range: Range,
        ) -> Self {
            Self(Node::raw(x, range))
        }
    }

    #[derive(Debug, PartialEq)]
    pub struct Declaration(
        pub Raw<super::Declaration<Binding, Expression, TypeExpression, Parameter, Module>>,
    );

    impl Declaration {
        pub const fn new(
            x: super::Declaration<Binding, Expression, TypeExpression, Parameter, Module>,
            range: Range,
        ) -> Self {
            Self(Node::raw(x, range))
        }
    }

    #[derive(Debug, PartialEq)]
    pub struct Import(pub Raw<super::Import>);

    impl Import {
        pub const fn new(x: super::Import, range: Range) -> Self {
            Self(Node::raw(x, range))
        }
    }

    #[derive(Debug, PartialEq)]
    pub struct Module(pub Raw<super::Module<Import, Declaration>>);

    impl Module {
        pub const fn new(x: super::Module<Import, Declaration>, range: Range) -> Self {
            Self(Node::raw(x, range))
        }
    }

    pub struct Program(pub Module);
}
