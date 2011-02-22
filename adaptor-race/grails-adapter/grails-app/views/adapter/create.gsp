

<%@ page import="com.ntkn.Adapter" %>
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
        <meta name="layout" content="main" />
        <g:set var="entityName" value="${message(code: 'adapter.label', default: 'Adapter')}" />
        <title><g:message code="default.create.label" args="[entityName]" /></title>
    </head>
    <body>
        <div class="nav">
            <span class="menuButton"><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></span>
            <span class="menuButton"><g:link class="list" action="list"><g:message code="default.list.label" args="[entityName]" /></g:link></span>
        </div>
        <div class="body">
            <h1><g:message code="default.create.label" args="[entityName]" /></h1>
            <g:if test="${flash.message}">
            <div class="message">${flash.message}</div>
            </g:if>
            <g:hasErrors bean="${adapterInstance}">
            <div class="errors">
                <g:renderErrors bean="${adapterInstance}" as="list" />
            </div>
            </g:hasErrors>
            <g:form action="save" >
                <div class="dialog">
                    <table>
                        <tbody>
                        
                            <tr class="prop">
                                <td valign="top" class="name">
                                    <label for="name"><g:message code="adapter.name.label" default="Name" /></label>
                                </td>
                                <td valign="top" class="value ${hasErrors(bean: adapterInstance, field: 'name', 'errors')}">
                                    <g:textField name="name" value="${adapterInstance?.name}" />
                                </td>
                            </tr>
                        
                            <tr class="prop">
                                <td valign="top" class="name">
                                    <label for="counter"><g:message code="adapter.counter.label" default="Counter" /></label>
                                </td>
                                <td valign="top" class="value ${hasErrors(bean: adapterInstance, field: 'counter', 'errors')}">
                                    <g:textField name="counter" value="${fieldValue(bean: adapterInstance, field: 'counter')}" />
                                </td>
                            </tr>
                        
                            <tr class="prop">
                                <td valign="top" class="name">
                                    <label for="delta"><g:message code="adapter.delta.label" default="Delta" /></label>
                                </td>
                                <td valign="top" class="value ${hasErrors(bean: adapterInstance, field: 'delta', 'errors')}">
                                    <g:textField name="delta" value="${fieldValue(bean: adapterInstance, field: 'delta')}" />
                                </td>
                            </tr>
                        
                            <tr class="prop">
                                <td valign="top" class="name">
                                    <label for="rank"><g:message code="adapter.rank.label" default="Rank" /></label>
                                </td>
                                <td valign="top" class="value ${hasErrors(bean: adapterInstance, field: 'rank', 'errors')}">
                                    <g:textField name="rank" value="${fieldValue(bean: adapterInstance, field: 'rank')}" />
                                </td>
                            </tr>
                        
                            <tr class="prop">
                                <td valign="top" class="name">
                                    <label for="time"><g:message code="adapter.time.label" default="Time" /></label>
                                </td>
                                <td valign="top" class="value ${hasErrors(bean: adapterInstance, field: 'time', 'errors')}">
                                    <g:textField name="time" value="${adapterInstance?.time}" />
                                </td>
                            </tr>
                        
                        </tbody>
                    </table>
                </div>
                <div class="buttons">
                    <span class="button"><g:submitButton name="create" class="save" value="${message(code: 'default.button.create.label', default: 'Create')}" /></span>
                </div>
            </g:form>
        </div>
    </body>
</html>
