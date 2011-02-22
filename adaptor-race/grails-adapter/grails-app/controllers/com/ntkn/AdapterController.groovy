package com.ntkn

class AdapterController {

    static allowedMethods = [save: "POST", update: "POST", delete: "POST"]

    def index = {
        redirect(action: "list", params: params)
    }

    def list = {
        params.max = Math.min(params.max ? params.int('max') : 10, 100)
        [adapterInstanceList: Adapter.list(params), adapterInstanceTotal: Adapter.count()]
    }

    def create = {
        def adapterInstance = new Adapter()
        adapterInstance.properties = params
        return [adapterInstance: adapterInstance]
    }

    def save = {
        def adapterInstance = new Adapter(params)
        if (adapterInstance.save(flush: true)) {
            flash.message = "${message(code: 'default.created.message', args: [message(code: 'adapter.label', default: 'Adapter'), adapterInstance.id])}"
            redirect(action: "show", id: adapterInstance.id)
        }
        else {
            render(view: "create", model: [adapterInstance: adapterInstance])
        }
    }

    def show = {
        def adapterInstance = Adapter.get(params.id)
        if (!adapterInstance) {
            flash.message = "${message(code: 'default.not.found.message', args: [message(code: 'adapter.label', default: 'Adapter'), params.id])}"
            redirect(action: "list")
        }
        else {
            [adapterInstance: adapterInstance]
        }
    }

    def edit = {
        def adapterInstance = Adapter.get(params.id)
        if (!adapterInstance) {
            flash.message = "${message(code: 'default.not.found.message', args: [message(code: 'adapter.label', default: 'Adapter'), params.id])}"
            redirect(action: "list")
        }
        else {
            return [adapterInstance: adapterInstance]
        }
    }

    def update = {
        def adapterInstance = Adapter.get(params.id)
        if (adapterInstance) {
            if (params.version) {
                def version = params.version.toLong()
                if (adapterInstance.version > version) {
                    
                    adapterInstance.errors.rejectValue("version", "default.optimistic.locking.failure", [message(code: 'adapter.label', default: 'Adapter')] as Object[], "Another user has updated this Adapter while you were editing")
                    render(view: "edit", model: [adapterInstance: adapterInstance])
                    return
                }
            }
            adapterInstance.properties = params
            if (!adapterInstance.hasErrors() && adapterInstance.save(flush: true)) {
                flash.message = "${message(code: 'default.updated.message', args: [message(code: 'adapter.label', default: 'Adapter'), adapterInstance.id])}"
                redirect(action: "show", id: adapterInstance.id)
            }
            else {
                render(view: "edit", model: [adapterInstance: adapterInstance])
            }
        }
        else {
            flash.message = "${message(code: 'default.not.found.message', args: [message(code: 'adapter.label', default: 'Adapter'), params.id])}"
            redirect(action: "list")
        }
    }

    def delete = {
        def adapterInstance = Adapter.get(params.id)
        if (adapterInstance) {
            try {
                adapterInstance.delete(flush: true)
                flash.message = "${message(code: 'default.deleted.message', args: [message(code: 'adapter.label', default: 'Adapter'), params.id])}"
                redirect(action: "list")
            }
            catch (org.springframework.dao.DataIntegrityViolationException e) {
                flash.message = "${message(code: 'default.not.deleted.message', args: [message(code: 'adapter.label', default: 'Adapter'), params.id])}"
                redirect(action: "show", id: params.id)
            }
        }
        else {
            flash.message = "${message(code: 'default.not.found.message', args: [message(code: 'adapter.label', default: 'Adapter'), params.id])}"
            redirect(action: "list")
        }
    }
}
