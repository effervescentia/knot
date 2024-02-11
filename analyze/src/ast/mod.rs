pub use lang::ast::*;

pub mod typed {
    use lang::{types, Node, Range};

    #[derive(Debug, PartialEq)]
    pub struct TypeRef<'a>(pub types::Type<&'a TypeRef<'a>>);

    type Typed<'a, Value> = Node<Value, TypeRef<'a>>;

    pub struct Binding(pub Node<super::Binding, ()>);

    impl Binding {
        pub const fn new(x: super::Binding, range: Range) -> Self {
            Self(Node::raw(x, range))
        }
    }

    pub struct Expression<'a>(
        pub Typed<'a, super::Expression<Expression<'a>, Statement<'a>, Component<'a>>>,
    );

    pub struct Statement<'a>(pub Typed<'a, super::Statement<Expression<'a>>>);

    pub struct Component<'a>(pub Typed<'a, super::Component<Component<'a>, Expression<'a>>>);

    pub struct TypeExpression<'a>(pub Typed<'a, super::TypeExpression<TypeExpression<'a>>>);

    pub struct Parameter<'a>(
        pub Typed<'a, super::Parameter<Binding, Expression<'a>, TypeExpression<'a>>>,
    );

    pub struct Declaration<'a>(
        pub  Typed<
            'a,
            super::Declaration<
                Binding,
                Expression<'a>,
                TypeExpression<'a>,
                Parameter<'a>,
                Module<'a>,
            >,
        >,
    );

    pub struct Import<'a>(pub Typed<'a, super::Import>);

    pub struct Module<'a>(pub Typed<'a, super::Module<Import<'a>, Declaration<'a>>>);

    pub struct Program<'a>(pub Module<'a>);
}
