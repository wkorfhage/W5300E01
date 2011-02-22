dataSource {
    pooled = true
    driverClassName = "com.mysql.jdbc.Driver"
    username = "lqu"
    password = "lqu"
}
hibernate {
    cache.use_second_level_cache = true
    cache.use_query_cache = true
    cache.provider_class = 'net.sf.ehcache.hibernate.EhCacheProvider'
}
// environment specific settings
environments {
    development {
        dataSource {
            dbCreate = "create" // one of 'create', 'create-drop','update'
            url = "jdbc:mysql://localhost/adapter"
        }
    }
    test {
        dataSource {
            dbCreate = "update"
            url = "jdbc:mysql://localhost/adapter"
        }
    }
    production {
        dataSource {
            dbCreate = "update"
            url = "jdbc:mysql://localhost/adapter"
        }
    }
}
