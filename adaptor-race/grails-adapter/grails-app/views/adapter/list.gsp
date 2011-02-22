
<%@ page import="com.ntkn.Adapter" %>
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
        <meta name="layout" content="main" />
        <g:set var="entityName" value="${message(code: 'adapter.label', default: 'Adapter')}" />
        <title><g:message code="default.list.label" args="[entityName]" /></title>
    </head>
    <body>
        <div class="nav">
            <span class="menuButton"><a class="home" href="${createLink(uri: '/')}"><g:message code="default.home.label"/></a></span>
            <span class="menuButton"><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></span>
        </div>
        <div class="body">
            <h1><g:message code="default.list.label" args="[entityName]" /></h1>
            <g:if test="${flash.message}">
            <div class="message">${flash.message}</div>
            </g:if>
            <div class="list">
                <table>
                    <thead>
                        <tr>
                        
                            <g:sortableColumn property="id" title="${message(code: 'adapter.id.label', default: 'Id')}" />
                        
                            <g:sortableColumn property="name" title="${message(code: 'adapter.name.label', default: 'Name')}" />
                        
                            <g:sortableColumn property="counter" title="${message(code: 'adapter.counter.label', default: 'Counter')}" />
                        
                            <g:sortableColumn property="delta" title="${message(code: 'adapter.delta.label', default: 'Delta')}" />
                        
                            <g:sortableColumn property="rank" title="${message(code: 'adapter.rank.label', default: 'Rank')}" />
                        
                            <g:sortableColumn property="time" title="${message(code: 'adapter.time.label', default: 'Time')}" />
                        
                        </tr>
                    </thead>
                    <tbody>
                    <g:each in="${adapterInstanceList}" status="i" var="adapterInstance">
                        <tr class="${(i % 2) == 0 ? 'odd' : 'even'}">
                        
                            <td><g:link action="show" id="${adapterInstance.id}">${fieldValue(bean: adapterInstance, field: "id")}</g:link></td>
                        
                            <td>${fieldValue(bean: adapterInstance, field: "name")}</td>
                        
                            <td>${fieldValue(bean: adapterInstance, field: "counter")}</td>
                        
                            <td>${fieldValue(bean: adapterInstance, field: "delta")}</td>
                        
                            <td>${fieldValue(bean: adapterInstance, field: "rank")}</td>
                        
                            <td>${fieldValue(bean: adapterInstance, field: "time")}</td>
                        
                        </tr>
                    </g:each>
                    </tbody>
                </table>
            </div>
            <div class="paginateButtons">
                <g:paginate total="${adapterInstanceTotal}" />
            </div>
        </div>
    </body>
</html>
