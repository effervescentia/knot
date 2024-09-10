use crate::{ast, Range};

pub trait CommonVisitor: Sized {
    type Context;
    type Binding;
    type TypeExpression;
    type Import;

    fn scoped<T, F>(self, f: F) -> (T, Self)
    where
        F: FnOnce(Self) -> (T, Self),
    {
        f(self)
    }

    fn binding(self, x: ast::Binding, r: Range) -> (Self::Binding, Self);

    fn type_expression(
        self,
        x: ast::TypeExpression<Self::Binding, Self::TypeExpression>,
        c: Self::Context,
    ) -> (Self::TypeExpression, Self);

    fn import(self, x: ast::Import, c: Self::Context) -> (Self::Import, Self);
}

pub trait ProgramVisitor: CommonVisitor {
    type Expression;
    type Statement;
    type Attribute;
    type Component;
    type Parameter;
    type Declaration;
    type Module;

    fn expression(
        self,
        x: ast::Expression<Self::Expression, Self::Statement, Self::Component>,
        c: Self::Context,
    ) -> (Self::Expression, Self);

    fn statement(
        self,
        x: ast::Statement<Self::Expression>,
        c: Self::Context,
    ) -> (Self::Statement, Self);

    fn attribute(
        self,
        x: ast::Attribute<Self::Expression>,
        c: Self::Context,
    ) -> (Self::Attribute, Self);

    fn component(
        self,
        x: ast::Component<Self::Component, Self::Expression, Self::Attribute>,
        c: Self::Context,
    ) -> (Self::Component, Self);

    fn parameter(
        self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        c: Self::Context,
    ) -> (Self::Parameter, Self);

    #[allow(clippy::type_complexity)]
    fn declaration(
        self,
        x: ast::Declaration<
            Self::Binding,
            Self::Expression,
            Self::TypeExpression,
            Self::Parameter,
            Self::Module,
        >,
        c: Self::Context,
    ) -> (Self::Declaration, Self);

    fn module(
        self,
        x: ast::Module<Self::Import, Self::Declaration>,
        c: Self::Context,
    ) -> (Self::Module, Self);
}

pub trait TypingsVisitor: CommonVisitor {
    type TypeDeclaration;
    type TypeModule;

    fn type_declaration(
        self,
        x: ast::TypeDeclaration<Self::Binding, Self::TypeExpression, Self::TypeModule>,
        c: Self::Context,
    ) -> (Self::TypeDeclaration, Self);

    fn type_module(
        self,
        x: ast::TypeModule<Self::Import, Self::TypeDeclaration>,
        c: Self::Context,
    ) -> (Self::TypeModule, Self);
}

pub trait Walk<Visitor> {
    type Output;

    fn walk(self, visitor: Visitor) -> (Self::Output, Visitor);
}

impl<Target, Visitor> Walk<Visitor> for (String, Target)
where
    Target: Walk<Visitor>,
{
    type Output = (String, Target::Output);

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (key, x) = self;
        let (x, v) = x.walk(v);

        ((key, x), v)
    }
}

impl<Target, Visitor> Walk<Visitor> for Option<Target>
where
    Target: Walk<Visitor>,
{
    type Output = Option<Target::Output>;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        if let Some(x) = self {
            let (x, v) = x.walk(v);
            (Some(x), v)
        } else {
            (None, v)
        }
    }
}

impl<Target, Visitor> Walk<Visitor> for Vec<Target>
where
    Target: Walk<Visitor>,
{
    type Output = Vec<Target::Output>;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.into_iter().fold((vec![], v), |(mut acc, v), x| {
            let (x, v) = x.walk(v);
            acc.push(x);
            (acc, v)
        })
    }
}

pub trait WalkEach<Visitor> {
    type Output;

    fn walk_each(self, visitor: Visitor) -> (Self::Output, Visitor);
}

impl<T1, T2, Visitor> WalkEach<Visitor> for (T1, T2)
where
    T1: Walk<Visitor>,
    T2: Walk<Visitor>,
{
    type Output = (T1::Output, T2::Output);

    fn walk_each(self, v: Visitor) -> (Self::Output, Visitor) {
        let (r1, v) = self.0.walk(v);
        let (r2, v) = self.1.walk(v);
        ((r1, r2), v)
    }
}

impl<T1, T2, T3, Visitor> WalkEach<Visitor> for (T1, T2, T3)
where
    T1: Walk<Visitor>,
    T2: Walk<Visitor>,
    T3: Walk<Visitor>,
{
    type Output = (T1::Output, T2::Output, T3::Output);

    fn walk_each(self, v: Visitor) -> (Self::Output, Visitor) {
        let ((r1, r2), v) = (self.0, self.1).walk_each(v);
        let (r3, v) = self.2.walk(v);
        ((r1, r2, r3), v)
    }
}

impl<T1, T2, T3, T4, Visitor> WalkEach<Visitor> for (T1, T2, T3, T4)
where
    T1: Walk<Visitor>,
    T2: Walk<Visitor>,
    T3: Walk<Visitor>,
    T4: Walk<Visitor>,
{
    type Output = (T1::Output, T2::Output, T3::Output, T4::Output);

    fn walk_each(self, v: Visitor) -> (Self::Output, Visitor) {
        let ((r1, r2, r3), v) = (self.0, self.1, self.2).walk_each(v);
        let (r4, v) = self.3.walk(v);
        ((r1, r2, r3, r4), v)
    }
}
