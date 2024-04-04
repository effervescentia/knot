use crate::{ast, Range};

pub type Span<Value, Meta> = (Value, (Range, Meta));

pub trait IntoSpan<Value, Meta> {
    fn into_span(self) -> Span<Value, Meta>;
}

impl<Value, Meta> IntoSpan<Value, Meta> for Span<Value, Meta> {
    fn into_span(self) -> Self {
        self
    }
}

pub trait Visit<Context>: Sized {
    type Binding;
    type Expression;
    type Statement;
    type Component;
    type TypeExpression;
    type Parameter;
    type Declaration;
    type Import;
    type Module;

    fn scoped<T, F>(self, f: F) -> (T, Self)
    where
        F: FnOnce(Self) -> (T, Self),
    {
        f(self)
    }

    fn binding(self, x: ast::Binding, r: Range) -> (Self::Binding, Self);

    fn expression(
        self,
        x: ast::Expression<Self::Expression, Self::Statement, Self::Component>,
        c: Context,
    ) -> (Self::Expression, Self);

    fn statement(self, x: ast::Statement<Self::Expression>, c: Context) -> (Self::Statement, Self);

    fn component(
        self,
        x: ast::Component<Self::Component, Self::Expression>,
        c: Context,
    ) -> (Self::Component, Self);

    fn type_expression(
        self,
        x: ast::TypeExpression<Self::TypeExpression>,
        c: Context,
    ) -> (Self::TypeExpression, Self);

    fn parameter(
        self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        c: Context,
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
        c: Context,
    ) -> (Self::Declaration, Self);

    fn import(self, x: ast::Import, c: Context) -> (Self::Import, Self);

    fn module(
        self,
        x: ast::Module<Self::Import, Self::Declaration>,
        c: Context,
    ) -> (Self::Module, Self);
}

pub trait Walk<Visitor, Context>
where
    Visitor: Visit<Context>,
{
    type Output;

    fn walk(self, visitor: Visitor) -> (Self::Output, Visitor);
}

impl<Target, Visitor, Context> Walk<Visitor, Context> for (String, Target)
where
    Target: Walk<Visitor, Context>,
    Visitor: Visit<Context>,
{
    type Output = (String, Target::Output);

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (key, x) = self;
        let (x, v) = x.walk(v);

        ((key, x), v)
    }
}

impl<Target, Visitor, Context> Walk<Visitor, Context> for Option<Target>
where
    Target: Walk<Visitor, Context>,
    Visitor: Visit<Context>,
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

impl<Target, Visitor, Context> Walk<Visitor, Context> for Vec<Target>
where
    Target: Walk<Visitor, Context>,
    Visitor: Visit<Context>,
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

pub trait WalkEach<Visitor, Context>
where
    Visitor: Visit<Context>,
{
    type Output;

    fn walk_each(self, visitor: Visitor) -> (Self::Output, Visitor);
}

impl<T1, T2, Visitor, Context> WalkEach<Visitor, Context> for (T1, T2)
where
    T1: Walk<Visitor, Context>,
    T2: Walk<Visitor, Context>,
    Visitor: Visit<Context>,
{
    type Output = (T1::Output, T2::Output);

    fn walk_each(self, v: Visitor) -> (Self::Output, Visitor) {
        let (r1, v) = self.0.walk(v);
        let (r2, v) = self.1.walk(v);
        ((r1, r2), v)
    }
}

impl<T1, T2, T3, Visitor, Context> WalkEach<Visitor, Context> for (T1, T2, T3)
where
    T1: Walk<Visitor, Context>,
    T2: Walk<Visitor, Context>,
    T3: Walk<Visitor, Context>,
    Visitor: Visit<Context>,
{
    type Output = (T1::Output, T2::Output, T3::Output);

    fn walk_each(self, v: Visitor) -> (Self::Output, Visitor) {
        let ((r1, r2), v) = (self.0, self.1).walk_each(v);
        let (r3, v) = self.2.walk(v);
        ((r1, r2, r3), v)
    }
}

impl<T1, T2, T3, T4, Visitor, Context> WalkEach<Visitor, Context> for (T1, T2, T3, T4)
where
    T1: Walk<Visitor, Context>,
    T2: Walk<Visitor, Context>,
    T3: Walk<Visitor, Context>,
    T4: Walk<Visitor, Context>,
    Visitor: Visit<Context>,
{
    type Output = (T1::Output, T2::Output, T3::Output, T4::Output);

    fn walk_each(self, v: Visitor) -> (Self::Output, Visitor) {
        let ((r1, r2, r3), v) = (self.0, self.1, self.2).walk_each(v);
        let (r4, v) = self.3.walk(v);
        ((r1, r2, r3, r4), v)
    }
}
